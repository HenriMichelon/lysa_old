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

    class AsyncQueue {
    public:

        struct Command {
            std::string location;
            vireo::CommandType commandType;
            std::shared_ptr<vireo::CommandList> commandList;
            std::shared_ptr<vireo::CommandAllocator> commandAllocator;
        };

        AsyncQueue(
            const std::shared_ptr<vireo::Vireo>& vireo,
            const std::shared_ptr<vireo::SubmitQueue>& transferQueue,
            const std::shared_ptr<vireo::SubmitQueue>& graphicQueue);

        void cleanup();

        Command beginCommand(vireo::CommandType commandType, const std::source_location& location = std::source_location::current());

        void endCommand(const Command& command, bool immediate = false);

        void submitCommands();

        std::shared_ptr<vireo::Buffer> createBuffer(
            const Command& command,
            vireo::BufferType type,
            size_t instanceSize,
            uint32 instanceCount);

    private:
        const std::shared_ptr<vireo::Vireo> vireo;

        // The submission thread & locks
        std::unique_ptr<std::thread> queueThread;
        std::mutex queueMutex;
        std::condition_variable queueCv;
        bool quit{false};

        std::mutex commandsMutex;
        std::unordered_map<vireo::CommandType, std::list<Command>> freeCommands;
        std::list<Command> commandsQueue;

        std::mutex buffersMutex;
        std::map<std::shared_ptr<vireo::CommandList>, std::list<std::shared_ptr<vireo::Buffer>>> buffers;

        Command previousCommand{};
        std::shared_ptr<vireo::SubmitQueue> transferQueue;
        std::shared_ptr<vireo::SubmitQueue> graphicQueue;
        std::shared_ptr<vireo::Fence> submitFence;

        void submit(const Command& command);

        void run();

    public:
        AsyncQueue(const AsyncQueue &) = delete;
        AsyncQueue &operator=(const AsyncQueue &) = delete;
    };


}
