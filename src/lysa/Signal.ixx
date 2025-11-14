/*
 * Copyright (c) 2025-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
export module lysa.signal;

import std;

export namespace lysa {

    /**
     * %Signal helper used by Object and engine nodes.
     *  - Store a list of handlers and invoke them when a signal is emitted.
     *  - Preserve the order of connections: handlers are called in the order they were connected.
     *
     * Notes:
     *  - Thread-safety: Signal is not thread-safe. If accessed from multiple threads,
     *    external synchronization is required around connect() and emit().
     *  - Lifetime: Handlers are stored by value (std::function). Ensure any captured
     *    objects outlive the signal emission or capture them safely by std::weak_ptr.
     *  - Disconnection: There is no explicit disconnect method at this time.
     */
    class Signal {
    public:
        /**
         * Type alias for a signal name used as a key by Object and others.
         */
        using signal = std::string;

        /**
         * Slot/handler signature invoked on emit().
         * Parameter is an optional opaque pointer forwarded by the emitter.
         */
        using Handler = std::function<void(void*)>;

        /**
         * Connects a handler to this signal.
         *
         * Handlers are appended and will be invoked in connection order when emit()
         * is called. The callable is copied into internal storage.
         */
        inline void connect(const Handler &handler) {
            handlers.push_back(handler);
        }

        /**
         * Emits the signal by invoking all connected handlers in connection order.
         *
         * Complexity is O(N) where N is the number of connected handlers. Any
         * exception thrown by a handler will propagate to the caller.
         *
         * @param params Optional opaque pointer forwarded to handlers (may be nullptr).
         */
        void emit(void* params) const;

    private:
        /**
         * Ordered list of handlers. std::list is used to preserve insertion order
         * and to keep references stable.
         */
        std::list<Handler> handlers{};
    };

}