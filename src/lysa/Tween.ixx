/*
 * Copyright (c) 2024-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
export module lysa.tween;

import std;
import lysa.enums;
import lysa.object;

export namespace lysa {

    /**
     * Base class for all tweeners classes.<br>
     * Tweens are objects that perform a specific animating task, e.g. interpolating a property of an Object. 
     */
    class Tween: public Object {
    public:
        using Callback = std::function<void()>;

        /**
         * Update the tween.
         * If the Tween have been created manually you need to call update() in your Node::onPhysicsProcess() function.
         * Do not call it if the Tween have been created with Node::create*Tween().
         * * @return `false` if the tween is running
         */
        virtual bool update(float deltaTime) = 0;

        auto isRunning() const { return running; }

        void kill() { running = false; }

    protected:
        bool running{false};
        Callback callback;
        TransitionType interpolationType;

        Tween(const TransitionType type, const Callback& callback):
            callback{callback}, interpolationType{type} {}
    };

    /**
     * Tween to interpolate a property of an Object.
     */
    template<typename T>
    class PropertyTween: public Tween {
    public:
        /**
         * A Setter method called by the Tween each update
         */
        typedef void (Object::*Setter)(const T&);

        /**
         * Create a Tween to tweens a property of an Object.
         * @param obj Target Object
         * @param set Setter to call on the Object
         * @param initial Initial value
         * @param final Final value
         * @param duration Animation duration in seconds
         * @param ttype Transition type
         * @param callback Callback called at the end of the animation
         */
        PropertyTween(Object* obj,
                      const Setter set,
                      T initial, 
                      T final, 
                      const float duration,
                      const TransitionType ttype = TransitionType::LINEAR,
                      const Callback& callback = nullptr):
            Tween{ttype, callback},
            durationTime{duration},
            targetValue{final},
            startValue{initial},
            targetObject{obj},
            setter{set} {}

         /**
         * Create a Tween to tweens a property of an Object.
         * @param obj Target Object
         * @param set Setter to call on the Object
         * @param initial Initial value
         * @param final Final value
         * @param duration Animation duration in seconds
         * @param ttype Transition type
         * @param callback Callback called at the end of the animation
         */
        PropertyTween(const std::shared_ptr<Object>& obj,
                      const Setter set,
                      T initial,
                      T final,
                      const float duration,
                      const TransitionType ttype = TransitionType::LINEAR,
                      const Callback& callback = nullptr):
            Tween{ttype, callback},
            durationTime{duration},
            targetValue{final},
            startValue{initial},
            targetObject{obj.get()},
            setter{set} {}

        /**
         * Interpolate the property.
         * If the Tween have been created manually you need to call update() in a Node::onPhysicsProcess() function.
         * *Do not call it* if the Tween have been created with Node::createPropertyTween().
         * @return `false` if the tween is running
         */
        bool update(const float deltaTime) override {
            elapsedTime += deltaTime;
            float t = std::min(elapsedTime / durationTime, 1.0f); // Normalized time
            (targetObject->*setter)(lerp(startValue, targetValue, t));
            running = (t < 1.0);
            if (!running && callback) {
                callback();
            }
            return !running;
        }

    private:
        float durationTime;
        float elapsedTime{0.0f};
        T targetValue;
        T startValue;
        Object* targetObject;
        Setter setter;
    };

}