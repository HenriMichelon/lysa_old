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

    SubmitQueue::SubmitQueue(
        const std::shared_ptr<vireo::Vireo>& vireo,
        const std::shared_ptr<vireo::SubmitQueue>& transferQueue,
        const std::shared_ptr<vireo::SubmitQueue>& graphicQueue) :
        mainThreadId{std::this_thread::get_id()},
        transferQueue{transferQueue},
        graphicQueue{graphicQueue},
        queueThread{&SubmitQueue::run, this} {
        submitFence =vireo->createFence();
    }

    void SubmitQueue::submit(const OneTimeCommand& command) {
        // INFO("queue on time command submit", command.location);
        const auto lock = std::lock_guard(getSubmitMutex());
        if (command.commandType == vireo::CommandType::TRANSFER) {
            transferQueue->submit(submitFence, { command.commandList });
        } else {
            graphicQueue->submit(submitFence, { command.commandList });
        }
        // wait the commands to be completed before destroying the command buffer
        submitFence->wait();
        submitFence->reset();
        const auto lock_commands = std::lock_guard(oneTimeMutex);
        oneTimeTransferCommands.push_back(command);
        {
            auto lockBuffer = std::lock_guard(oneTimeBuffersMutex);
            oneTimeBuffers.erase(command.commandList);
        }
    }

    std::shared_ptr<vireo::Buffer> SubmitQueue::createOneTimeBuffer(
        const OneTimeCommand& oneTimeCommand,
        const vireo::BufferType type,
        const size_t instanceSize,
        const uint32 instanceCount) {
        // INFO("TransferQueue::createOneTimeBuffer ", std::to_string(static_cast<int>(type)));
        auto lock = std::lock_guard(oneTimeBuffersMutex);
        oneTimeBuffers[oneTimeCommand.commandList].emplace_back(Application::getVireo().createBuffer(type, instanceSize, instanceCount));
        return oneTimeBuffers[oneTimeCommand.commandList].back();
    }

    SubmitQueue::OneTimeCommand SubmitQueue::beginOneTimeGraphicCommand(const std::source_location& location) {
        auto lock = std::lock_guard(oneTimeMutex);
        if (oneTimeGraphicCommands.empty()) {
            const auto commandAllocator = Application::getVireo().createCommandAllocator(
                vireo::CommandType::GRAPHIC);
            const auto commandList = commandAllocator->createCommandList();
            commandList->begin();
            std::stringstream ss;
            ss << location.function_name() << " line " << location.line();
            return {vireo::CommandType::GRAPHIC, ss.str(), commandAllocator, commandList};
        }
        const auto command = oneTimeGraphicCommands.front();
        oneTimeGraphicCommands.pop_front();
        command.commandAllocator->reset();
        command.commandList->begin();
        return command;
    }

    SubmitQueue::OneTimeCommand SubmitQueue::beginOneTimeTransferCommand(const std::source_location& location) {
        auto lock = std::lock_guard(oneTimeMutex);
        if (oneTimeTransferCommands.empty()) {
            const auto commandAllocator = Application::getVireo().createCommandAllocator(
                vireo::CommandType::TRANSFER);
            const auto commandList = commandAllocator->createCommandList();
            commandList->begin();
            std::stringstream ss;
            ss << location.function_name() << " line " << location.line();
            return { vireo::CommandType::TRANSFER, ss.str(), commandAllocator, commandList};
        }
        const auto command = oneTimeTransferCommands.front();
        oneTimeTransferCommands.pop_front();
        command.commandAllocator->reset();
        command.commandList->begin();
        return command;
    }

    void SubmitQueue::endOneTimeCommand(const OneTimeCommand& oneTimeCommand, const bool immediate) {
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

    void SubmitQueue::run() {
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

    void SubmitQueue::stop() {
        quit = true;
        queueCv.notify_one();
        queueThread.join();
        for (auto& command : oneTimeTransferCommands) {
            command.commandList.reset();
            command.commandAllocator.reset();
        }
        for (auto& command : oneTimeGraphicCommands) {
            command.commandList.reset();
            command.commandAllocator.reset();
        }
        submitFence.reset();
    }

}