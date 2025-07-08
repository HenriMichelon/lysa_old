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

    TransferQueue::TransferQueue(
        const std::shared_ptr<vireo::Vireo>& vireo,
        const vireo::CommandType queueType,
        const std::shared_ptr<vireo::SubmitQueue>& transferQueue) :
        mainThreadId{std::this_thread::get_id()},
        queueType{queueType},
        transferQueue{transferQueue},
        queueThread{&TransferQueue::run, this} {
        submitFence =vireo->createFence();
    }

    void TransferQueue::submit(const OneTimeCommand& command) {
        INFO("queue on time command submit", command.location);
        const auto lock = std::lock_guard(getSubmitMutex());
        transferQueue->submit(submitFence, { command.commandList });
        // wait the commands to be completed before destroying the command buffer
        submitFence->wait();
        submitFence->reset();
        const auto lock_commands = std::lock_guard(oneTimeMutex);
        oneTimeCommands.push_back(command);
        {
            auto lockBuffer = std::lock_guard(oneTimeBuffersMutex);
            oneTimeBuffers.erase(command.commandList);
        }
    }

    std::shared_ptr<vireo::Buffer> TransferQueue::createOneTimeBuffer(
        const OneTimeCommand& oneTimeCommand,
        const vireo::BufferType type,
        const size_t instanceSize,
        const uint32 instanceCount) {
        INFO("TransferQueue::createOneTimeBuffer ", oneTimeCommand.location);
        auto lock = std::lock_guard(oneTimeBuffersMutex);
        oneTimeBuffers[oneTimeCommand.commandList].emplace_back(Application::getVireo().createBuffer(type, instanceSize, instanceCount));
        return oneTimeBuffers[oneTimeCommand.commandList].back();
    }

    TransferQueue::OneTimeCommand TransferQueue::beginOneTimeCommand(const std::source_location& location) {
        auto lock = std::lock_guard(oneTimeMutex);
        if (oneTimeCommands.empty()) {
            const auto commandAllocator = Application::getVireo().createCommandAllocator(queueType);
            const auto commandList = commandAllocator->createCommandList();
            commandList->begin();
            std::stringstream ss;
            ss << location.function_name() << " line " << location.line();
            return {ss.str(), commandAllocator, commandList};
        }
        const auto command = oneTimeCommands.front();
        oneTimeCommands.pop_front();
        command.commandAllocator->reset();
        command.commandList->begin();
        return command;
    }

    void TransferQueue::endOneTimeCommand(const OneTimeCommand& oneTimeCommand, const bool immediate) {
        oneTimeCommand.commandList->end();
        if (immediate) {
            auto lock = std::lock_guard{queueMutex};
            submit(oneTimeCommand);
        } else {
            auto lock = std::lock_guard{queueMutex};
            commands.push_back(oneTimeCommand);
            queueCv.notify_one();
        }
    }

    void TransferQueue::run() {
        while (!quit) {
            auto lock = std::unique_lock{queueMutex};
            queueCv.wait(lock, [this] {
                return quit || !commands.empty();
            });
            if (quit) { break; }
            auto command = commands.front();
            commands.pop_front();
            submit(command);
        }
    }

    void TransferQueue::stop() {
        quit = true;
        queueCv.notify_one();
        queueThread.join();
        for (auto& command : oneTimeCommands) {
            command.commandList.reset();
            command.commandAllocator.reset();
        }
        submitFence.reset();
    }

}