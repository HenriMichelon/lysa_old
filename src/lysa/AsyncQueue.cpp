/*
* Copyright (c) 2024-present Henri Michelon
 *
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module lysa.transfer_queue;

import lysa.application;
import lysa.log;

namespace lysa {

    AsyncQueue::AsyncQueue(
        const std::shared_ptr<vireo::Vireo>& vireo,
        const std::shared_ptr<vireo::SubmitQueue>& transferQueue,
        const std::shared_ptr<vireo::SubmitQueue>& graphicQueue) :
        vireo{vireo},
        transferQueue{transferQueue},
        graphicQueue{graphicQueue} {
        submitFence = vireo->createFence(true);
    }

    void AsyncQueue::submitCommands() {
        if (!commandsQueue.empty()) {
            auto lockCommands = std::lock_guard(commandsMutex);
            // INFO("command submit ", commandsQueue.size());
            // for (auto& command : commandsQueue) {
            //     submit(command);
            // }
            // commandsQueue.clear();
            const auto command = commandsQueue.front();
            commandsQueue.pop_front();
            submit(command);
        }
    }

    void AsyncQueue::submit(const Command& command) {
        submitFence->wait();
        submitFence->reset();
        if (previousCommand.commandList != nullptr) {
            auto lockBuffer = std::lock_guard(buffersMutex);
            buffers.erase(previousCommand.commandList);
            freeCommands[previousCommand.commandType].push_back(previousCommand);
        }
        if (command.commandType == vireo::CommandType::GRAPHIC) {
            graphicQueue->submit(submitFence, { command.commandList });
        } else {
            transferQueue->submit(submitFence, { command.commandList });
        }
        previousCommand = command;
    }

    std::shared_ptr<vireo::Buffer> AsyncQueue::createBuffer(
        const Command& command,
        const vireo::BufferType type,
        const size_t instanceSize,
        const uint32 instanceCount) {
        // INFO("TransferQueue::createOneTimeBuffer ", std::to_string(static_cast<int>(type)));
        auto lock = std::lock_guard(buffersMutex);
        buffers[command.commandList].emplace_back(vireo->createBuffer(type, instanceSize, instanceCount));
        return buffers[command.commandList].back();
    }

    AsyncQueue::Command AsyncQueue::beginCommand(const vireo::CommandType commandType, const std::source_location& location) {
        auto lock = std::lock_guard(commandsMutex);
        if (freeCommands[commandType].empty()) {
            const auto commandAllocator = vireo->createCommandAllocator(commandType);
            const auto commandList = commandAllocator->createCommandList();
            commandList->begin();
            std::stringstream ss;
            ss << location.function_name() << " line " << location.line();
            return {ss.str(), commandType, commandList, commandAllocator};
        }
        const auto command = freeCommands[commandType].front();
        freeCommands[commandType].pop_front();
        command.commandAllocator->reset();
        command.commandList->begin();
        return command;
    }

    void AsyncQueue::endCommand(const Command& command, const bool immediate) {
        command.commandList->end();
        if (immediate) {
            submit(command);
        } else {
            auto lock = std::lock_guard{commandsMutex};
            commandsQueue.push_back(command);
        }
    }

    void AsyncQueue::cleanup() {
        submitFence.reset();
        for (auto& command : commandsQueue) {
            command.commandList.reset();
            command.commandAllocator.reset();
        }
        for (auto& [type, commands] : freeCommands) {
            for (auto& command : commands) {
                command.commandList.reset();
                command.commandAllocator.reset();
            }
        }
    }

}