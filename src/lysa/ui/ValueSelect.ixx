/*
 * Copyright (c) 2024-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
export module lysa.ui.value_select;

import lysa.ui.widget;

export namespace lysa::ui {

    class ValueSelect : public Widget {
    public:
        ValueSelect(
            Type type,
            float min,
            float max,
            float value,
            float step);

        auto getMin() const { return min; }

        auto getMax() const { return max; }

        auto getValue() const { return value; }

        auto getStep() const { return step; }

        virtual void setMin(float min);

        virtual void setMax(float max);

        virtual void setValue(float value);

        virtual void setStep(float step);

    protected:
        float min;
        float max;
        float value;
        float step;

        void eventResize() override;

        virtual void eventRangeChange();

        virtual void eventValueChange(float prev);
    };
}
