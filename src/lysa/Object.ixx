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
     * Base class for anything
     */
    class Object {
    public:
        /**
         * Connects a signal by name to a function
         * @param name signal name.
         * @param handler the member function to call when emit() is called
        */
        void connect(const Signal::signal &name, const Signal::Handler& handler);

        /**
         * Connects a signal by name to a function
         * @param name signal name.
         * @param handler the member function to call when emit() is called
        */
        void connect(const Signal::signal &name, const std::function<void()>& handler);

        /**
         * Emits a signal by name by calling all the connected function in the connect order
         * @param name signal name
         * @param params parameters to pass to the function connected to the signal
         */
        void emit(const Signal::signal &name, void *params = nullptr);

        /**
         * Converts the objet to a readable text
         */
        virtual std::string toString() const { return "Object"; }

        friend std::ostream &operator<<(std::ostream &os, const Object &obj) {
            os << obj.toString();
            return os;
        }

        Object() = default;
        virtual ~Object() = default;

    private:
        std::map<std::string, Signal> signals;
    };

    class Updatable {
    public:
        auto isUpdated() const { return pendingUpdates > 0;}

        void decrementUpdates() { pendingUpdates -= 1; }

        void setUpdated() { pendingUpdates = maxUpdates; }

        void setMaxUpdates(const uint32 maxUpdates) { this->maxUpdates = maxUpdates; }

        uint32 pendingUpdates{0};
        uint32 maxUpdates{0};
    };

}
