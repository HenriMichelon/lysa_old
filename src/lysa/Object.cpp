/*
 * Copyright (c) 2025-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module lysa.object;

namespace lysa {

    void Object::connect(const Signal::signal &name, const Signal::Handler& handler) {
        signals[name].connect(handler);
    }

    void Object::connect(const Signal::signal &name, const std::function<void()>& handler) {
        signals[name].connect([handler](void*) {
            handler();
        });
    }

    void Object::emit(const Signal::signal &name, void *params) {
        if (signals.contains(name)) {
            signals[name].emit(params);
        }
    }

}
