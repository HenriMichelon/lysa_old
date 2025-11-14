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

    /**
     * %A lightweight background submission system used by Lysa to build and submit
     * GPU command lists asynchronously. It owns a worker thread that drains a queue
     * of recorded commands and submits them to the appropriate Vireo submit queue
     * (transfer or graphic), allowing asset uploads and simple GPU work to happen
     * without stalling the main thread.
     *
     * Typical usage:
     *  - Call beginCommand() to acquire a command list for a specific CommandType.
     *  - Record GPU work (e.g., copies, barriers) on the returned command list.
     *  - Call endCommand() to enqueue the work; optionally set immediate=true to
     *    trigger an immediate submit when appropriate.
     *  - The internal worker thread will batch and submit pending commands.
     *
     * Thread-safety:
     *  - Public methods are thread-safe unless otherwise specified. Internals are
     *    protected via mutexes. Commands and temporary buffers are tracked per
     *    command list to avoid premature destruction.
     */
    class AsyncQueue {
    public:

        /**
         * Holds resources necessary to record and submit a single unit of work.
         */
        struct Command {
            /// Call-site used for debugging and profiling (file/function/line).
            std::string location;
            /// Target queue type this command will be submitted to.
            vireo::CommandType commandType;
            /// Command list used to record GPU work.
            std::shared_ptr<vireo::CommandList> commandList;
            /// Allocator from which the command list is reset/allocated.
            std::shared_ptr<vireo::CommandAllocator> commandAllocator;
        };

        /**
         * Construct an asynchronous queue manager bound to Vireo and two submit
         * queues (transfer and graphic).
         */
        AsyncQueue(
            const std::shared_ptr<vireo::Vireo>& vireo,
            const std::shared_ptr<vireo::SubmitQueue>& transferQueue,
            const std::shared_ptr<vireo::SubmitQueue>& graphicQueue);

        /**
         * Release background resources and flush outstanding commands. Safe to
         * call during application shutdown.
         */
        void cleanup();

        /**
         * Acquire a command for the specified queue type and begin recording on
         * its command list. The source_location is captured by default for
         * debugging purposes.
         */
        Command beginCommand(vireo::CommandType commandType, const std::source_location& location = std::source_location::current());

        /**
         * Finish recording and enqueue the command for submission. If immediate is
         * `true`, the queue may attempt to submit pending commands right away.
         */
        void endCommand(const Command& command, bool immediate = false);

        /**
         * Trigger submission of all pending commands. Usually invoked by the
         * engine tick; the background thread will also submit when woken.
         */
        void submitCommands();

        /**
         * Create a buffer that is tracked alongside the provided command so the
         * resource remains alive until the GPU work referencing it has been
         * submitted. Useful for transient staging buffers.
         */
        std::shared_ptr<vireo::Buffer> createBuffer(
            const Command& command,
            vireo::BufferType type,
            size_t instanceSize,
            uint32 instanceCount);

    private:
        // Backend entry point used to create GPU objects and fences.
        const std::shared_ptr<vireo::Vireo> vireo;

        // Background worker that drains and submits queued commands.
        std::unique_ptr<std::thread> queueThread;
        // Protects condition variable and quit flag.
        std::mutex queueMutex;
        // Signals the worker thread to wake up for submission or shutdown.
        std::condition_variable queueCv;
        // Set to true to request the worker thread to exit.
        bool quit{false};

        // Protects freeCommands and commandsQueue.
        std::mutex commandsMutex;
        // Pools of reusable Command objects indexed by command type.
        std::unordered_map<vireo::CommandType, std::list<Command>> freeCommands;
        // FIFO of commands ready to be submitted by the worker.
        std::list<Command> commandsQueue;

        // Protects buffers map.
        std::mutex buffersMutex;
        // Transient buffers that must stay alive until the associated command list is submitted.
        std::map<std::shared_ptr<vireo::CommandList>, std::list<std::shared_ptr<vireo::Buffer>>> buffers;

        // Last command begun; used to chain or coalesce submissions when needed.
        Command previousCommand{};
        // Target submit queue for transfer operations (uploads/copies).
        std::shared_ptr<vireo::SubmitQueue> transferQueue;
        // Target submit queue for graphics operations.
        std::shared_ptr<vireo::SubmitQueue> graphicQueue;
        // Fence used to synchronize submissions and resource lifetimes.
        std::shared_ptr<vireo::Fence> submitFence;

        // Internal helper that performs the actual submit for a given command.
        void submit(const Command& command);

        // Worker thread main loop.
        void run();

    public:
        AsyncQueue(const AsyncQueue &) = delete;
        AsyncQueue &operator=(const AsyncQueue &) = delete;
    };


}
