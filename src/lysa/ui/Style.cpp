/*
 * Copyright (c) 2024-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module lysa.ui.style;

import lysa.exception;
import lysa.types;
import lysa.ui.style_classic;

namespace lysa::ui {

    Style::Style() { font = nullptr; }

    std::shared_ptr<Style> Style::create(const std::string &name) {
        std::shared_ptr<Style> style;
        if (name == "vector") {
            style = std::make_shared<StyleClassic>();
        }
        /*else if (NAME == "pixmap") {
            tmp = (GLayout*) new GLayoutPixmap;
        }*/
        if (!style) { throw Exception("No style named ", name); }
        style->init();
        style->updateOptions();
        return style;
    }

    void Style::setOption(const std::string &name, const std::string &value) {
        std::shared_ptr<StyleOption> option;
        for (const auto &opt : options) {
            if (opt->name == name) {
                option = opt;
                break;
            }
        }
        if (option == nullptr) {
            option = make_shared<StyleOption>(name);
            options.push_back(option);
        }
        option->value = value;
        updateOptions();
    }

    std::string Style::getOption(const std::string &name) const {
        std::shared_ptr<StyleOption> option;
        for (const auto &opt : options) {
            if (opt->name == name) {
                option = opt;
                break;
            }
        }
        if (option == nullptr) {
            return "";
        }
        return option->value;
    }
}
