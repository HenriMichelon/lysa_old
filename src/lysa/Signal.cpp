/*
 * Copyright (c) 2025-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module lysa.signal;

namespace lysa {

    void Signal::emit(void *params) const {
        for (const auto &callable : handlers) {
            callable(params);
        }
    }

}
