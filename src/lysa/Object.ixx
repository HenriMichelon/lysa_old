/*
 * Copyright (c) 2025-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
export module lysa.object;

import std;
import lysa.signal;
import lysa.types;

export namespace lysa {

    /**
     * Base class providing a minimal signal/slot facility and a
     * textual representation.
     *  - Allow users to connect named signals to handlers (slots).
     *  - Emit signals by name, invoking all connected handlers in connection order.
     *  - Provide a virtual toString() for debugging/logging and stream insertion.
     *
     * Notes:
     *  - Signal handlers are stored per signal name; multiple handlers per name
     *    are supported and are invoked in the order they were connected.
     *  - This class does not manage thread-safety; external synchronization is
     *    required if used across threads.
     */
    class Object {
    public:
        /**
         * Connects a signal by name to a member/function handler with parameters.
         *
         * The concrete handler signature is defined by Signal::Handler and allows
         * passing an optional opaque pointer when emitting.
         *
         * @param name Signal name to bind to.
         * @param handler Callable to invoke when emit() is called with the same name.
         */
        void connect(const Signal::signal &name, const Signal::Handler& handler);

        /**
         * Connects a signal by name to a parameterless callable.
         *
         * This overload is convenient for simple notifications that do not
         * consume the params pointer passed to emit().
         *
         * @param name Signal name to bind to.
         * @param handler Callable with no arguments invoked on emit().
         */
        void connect(const Signal::signal &name, const std::function<void()>& handler);

        /**
         * Emits a signal by name, invoking all connected handlers in connection order.
         *
         * Handlers registered via the Signal::Handler overload receive the params
         * pointer as provided here. Handlers registered via the parameterless
         * overload are called without arguments.
         *
         * @param name Signal name to emit.
         * @param params Optional opaque pointer forwarded to handlers (may be nullptr).
         */
        void emit(const Signal::signal &name, void *params = nullptr);

        /**
         * Returns a human-readable representation of the object for logging/debugging.
         * Override in derived types to provide more context.
         */
        virtual std::string toString() const { return "Object"; }

        /**
         * Stream insertion operator that forwards to toString().
         */
        friend std::ostream &operator<<(std::ostream &os, const Object &obj) {
            os << obj.toString();
            return os;
        }

        Object() = default;
        virtual ~Object() = default;

    private:
        /** Map of signal name to its Signal container (list of handlers). */
        std::map<std::string, Signal> signals;
    };

    /**
     * Helper mixin that tracks a finite number of pending updates for an object.
     * Used by the engine to update resources in VRAM and associated pipelines and descriptors.
     *
     * Typical usage is to mark an object as needing refresh a limited number of
     * frames, decrementing the counter as work is performed, and querying the
     * updated state with isUpdated().
     */
    class Updatable {
    public:
        /** Returns true if there are pending updates to process. */
        auto isUpdated() const { return pendingUpdates > 0;}

        /** Decrements the number of pending updates by one (not clamped). */
        void decrementUpdates() { pendingUpdates -= 1; }

        /** Marks the object as updated, respecting maxUpdates if set. */
        void setUpdated();

        /** Sets the maximum number of consecutive pending updates allowed. */
        void setMaxUpdates(const uint32 maxUpdates) { this->maxUpdates = maxUpdates; }

        /** Current number of pending updates to process. */
        uint32 pendingUpdates{0};
        /** Upper bound on pendingUpdates; 0 means unbounded. */
        uint32 maxUpdates{0};
    };

}
