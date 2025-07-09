/*
 * Copyright (c) 2024-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
export module lysa.transfer_queue;

import std;
import vireo;
import lysa.types;

export namespace lysa {

    class SubmitQueue {
    public:

        struct OneTimeCommand {
            vireo::CommandType commandType;
            const std::string location;
            std::shared_ptr<vireo::CommandAllocator> commandAllocator;
            std::shared_ptr<vireo::CommandList> commandList;
        };

        SubmitQueue(
            const std::shared_ptr<vireo::Vireo>& vireo,
            const std::shared_ptr<vireo::SubmitQueue>& transferQueue,
            const std::shared_ptr<vireo::SubmitQueue>& graphicQueue);

        void stop();

        auto& getSubmitMutex() { return submitMutex; }

        OneTimeCommand beginOneTimeTransferCommand(const std::source_location& location = std::source_location::current());
        OneTimeCommand beginOneTimeGraphicCommand(const std::source_location& location = std::source_location::current());

        void endOneTimeCommand(const OneTimeCommand& oneTimeCommand, bool immediate = false);

        std::shared_ptr<vireo::Buffer> createOneTimeBuffer(
            const OneTimeCommand& oneTimeCommand,
            vireo::BufferType type,
            size_t instanceSize,
            uint32 instanceCount);

    private:
        std::thread::id mainThreadId;
        // Queues to submit commands to the GPU
        std::shared_ptr<vireo::SubmitQueue> transferQueue;
        std::shared_ptr<vireo::SubmitQueue> graphicQueue;
        // Stop the queue thread
        bool quit{false};
        // Submission queue
        std::list<OneTimeCommand> commands;
        // To synchronize between the main thread & submit thread
        std::mutex submitMutex;
        std::shared_ptr<vireo::Fence> submitFence;

        // The submission thread & locks
        std::thread queueThread;
        std::mutex queueMutex;
        std::condition_variable queueCv;

        // Temporary one-time command buffers, associated buffers and command pools
        // One command pool per command buffer
        std::list<OneTimeCommand> oneTimeTransferCommands;
        std::list<OneTimeCommand> oneTimeGraphicCommands;
        std::mutex oneTimeMutex;
        std::mutex oneTimeBuffersMutex;
        std::map<std::shared_ptr<vireo::CommandList>, std::list<std::shared_ptr<vireo::Buffer>>> oneTimeBuffers;

        void run();

        void submit(const OneTimeCommand& command);

    public:
        SubmitQueue(const SubmitQueue &) = delete;
        SubmitQueue &operator=(const SubmitQueue &) = delete;
    };


}
