/*
 * Copyright (c) 2024-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module lysa.ui.frame;

namespace lysa::ui {

    void Frame::setTitle(const std::string& title) {
        this->title = title;
        resizeChildren();
        refresh();
    }

}
