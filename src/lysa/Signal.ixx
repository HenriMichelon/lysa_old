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
     * %A signal of an Object
     */
    class Signal {
    public:
        /**
        * Name of a signal
        */
        using signal = std::string;

        /**
         * Function who answers to emitted signals
         */
        using Handler = std::function<void(void*)>;

        /**
         * Connects a function to the signal
        */
        inline void connect(const Handler &handler) {
            handlers.push_back(handler);
        }

        /**
         * Emits the signal by calling all the connected functions in the connect order
         * @param params parameters to pass to the function connected to the signal
         */
        void emit(void* params) const;

    private:
        std::list<Handler> handlers{};
    };

}