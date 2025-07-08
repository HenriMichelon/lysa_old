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

    class TransferQueue {
    public:

        struct OneTimeCommand {
            const std::string location;
            std::shared_ptr<vireo::CommandAllocator> commandAllocator;
            std::shared_ptr<vireo::CommandList> commandList;
        };

        TransferQueue(const std::shared_ptr<vireo::Vireo>& vireo, vireo::CommandType queueType, const std::shared_ptr<vireo::SubmitQueue>& transferQueue);

        void stop();

        auto& getSubmitMutex() { return submitMutex; }

        OneTimeCommand beginOneTimeCommand(const std::source_location& location = std::source_location::current());

        void endOneTimeCommand(const OneTimeCommand& oneTimeCommand, bool immediate = false);

        std::shared_ptr<vireo::Buffer> createOneTimeBuffer(
            const OneTimeCommand& oneTimeCommand,
            vireo::BufferType type,
            size_t instanceSize,
            uint32 instanceCount);

    private:
        std::thread::id mainThreadId;
        vireo::CommandType queueType;
        // Queue to submit commands to the GPU
        std::shared_ptr<vireo::SubmitQueue> transferQueue;
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
        std::list<OneTimeCommand> oneTimeCommands;
        std::mutex oneTimeMutex;
        std::mutex oneTimeBuffersMutex;
        std::map<std::shared_ptr<vireo::CommandList>, std::list<std::shared_ptr<vireo::Buffer>>> oneTimeBuffers;

        void run();

        void submit(const OneTimeCommand& command);

    public:
        TransferQueue(const TransferQueue &) = delete;
        TransferQueue &operator=(const TransferQueue &) = delete;
    };


}
