/*
 * Copyright (c) 2024-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module lysa.ui.value_select;

import lysa.exception;
import lysa.ui.event;

namespace lysa::ui {

    ValueSelect::ValueSelect(
        const Type type,
        const float min,
        const float max,
        const float value,
        const float step):
        Widget{type},
        min{min},
        max{max},
        value{value},
        step{step} {
    }

    void ValueSelect::setMin(const float min) {
        if (this->min == min) return;
        if (min > max) return;
        this->min = min;
        if (value < min) {
            setValue(min);
        }
        resizeChildren();
        eventRangeChange();
        refresh();
        auto event = EventRange{.min = min, .max = max, .value = value};
        emit(Event::OnRangeChange, &event);
    }

    void ValueSelect::setMax(const float max) {
        if (this->max == max) return;
        if (max < min) return;
        this->max = max;
        if (value > max) {
            setValue(max);
        }
        resizeChildren();
        eventRangeChange();
        auto event = EventRange{.min = min, .max = max, .value = value};
        emit(Event::OnRangeChange, &event);
    }

    void ValueSelect::setValue(const float value) {
        if (this->value == value) { return; }
        const float prev = value;
        this->value = value;
        if (this->value < min) {
            this->value = min;
        }
        if (this->value > max) {
            this->value = max;
        }
        eventRangeChange();
        refresh();
        if (parent) {
            parent->refresh();
        }
        auto event = EventValue{.value = this->value, .previous = prev};
        emit(Event::OnValueChange, &event);
    }

    void ValueSelect::setStep(const float step) {
        if (this->step == step) return;
        assert([&] { return step != 0;}, "ValueSelect: can't use a step of 0");
        this->step = step;
        eventRangeChange();
        refresh();
    }

    void ValueSelect::eventResize() {
        Widget::eventResize();
        eventRangeChange();
    }

    void ValueSelect::eventRangeChange() {
        auto event = EventRange{.min = min, .max = max, .value = value};
        emit(Event::OnRangeChange, &event);
    }

    void ValueSelect::eventValueChange(const float prev) {
        auto event = EventValue{.value = value, .previous = prev};
        emit(Event::OnValueChange, &event);
    }

}
