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
     * Base class for time-based animations (tweens).
     * Tweens encapsulate a small, focused animation task that progresses with
     * time, typically driven every physics tick. Common examples include
     * interpolating a property value on a target Object over a given duration,
     * then optionally invoking a completion callback.
     *
     * Usage:
     *  - If a Tween is created manually, call update(deltaTime) from
     *    Node::onPhysicsProcess() to advance it.
     *  - If a Tween is created through Node::create*Tween() helpers, the engine
     *    will update it automatically; do not call update() yourself in that case.
     *
     * Notes:
     *  - All tweens expose a running state that becomes `false` once the animation
     *    completes. The update() method conventionally returns `true` when the
     *    tween is finished and false while it is still running.
     *  - The TransitionType controls the interpolation curve. Concrete subclasses are responsible for applying it.
     */
    class Tween: public Object {
    public:
        /** Callback invoked when the tween completes (optional). */
        using Callback = std::function<void()>;

        /**
         * Advances the tween by delta time.
         *
         * Call this from Node::onPhysicsProcess() when the tween has been
         * created manually. Do not call it for tweens created via
         * Node::create*Tween(), as they are advanced by the engine.
         *
         * @param deltaTime Time elapsed since the previous update (seconds).
         * @return true when the tween has finished, false while it is running.
         */
        virtual bool update(float deltaTime) = 0;

        /** Returns true while the tween is still animating. */
        auto isRunning() const { return running; }

        /** Immediately stops the tween without firing the completion callback. */
        void kill() { running = false; }

    protected:
        /** True while the tween is active. Set to false upon completion or kill(). */
        bool running{false};
        /** Optional completion callback invoked when the tween ends naturally. */
        Callback callback;
        /** Interpolation curve applied by the tween (e.g., linear, ease-in/out). */
        TransitionType interpolationType;

        /**
         * Constructs a tween with the given interpolation type and optional callback.
         * @param type Interpolation curve.
         * @param callback Function invoked on completion (may be nullptr).
         */
        Tween(const TransitionType type, const Callback& callback):
            callback{callback}, interpolationType{type} {}
    };

    /**
     * Tween that interpolates a property of an Object via a setter function.
     *
     * The template parameter T is the property value type (e.g., float, float3).
     * Each update computes the interpolated value and calls the provided setter
     * on the target object. When the duration elapses, the tween stops and the
     * optional completion callback is invoked.
     */
    template<typename T>
    class PropertyTween: public Tween {
    public:
        /**
         * Pointer-to-member setter method called every update.
         * The signature must accept a const T&.
         */
        typedef void (Object::*Setter)(const T&);

        /**
         * Creates a tween that interpolates a property on an Object.
         *
         * @param obj      Target object whose property will be animated.
         * @param set      Member setter to call on the target object.
         * @param initial  Starting value applied at time 0.
         * @param final    Target value reached at the end of the duration.
         * @param duration Animation duration in seconds (must be > 0).
         * @param ttype    Interpolation curve to use (default: LINEAR).
         * @param callback Optional function invoked when the tween completes.
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
        * Creates a tween that interpolates a property on an Object.
        * This overload accepts a shared_ptr to the target; the pointer is not
        * retained beyond construction (the raw pointer is stored), so callers
        * must ensure the target outlives the tween.
        *
        * @param obj      Shared pointer to the target object.
        * @param set      Member setter to call on the target object.
        * @param initial  Starting value applied at time 0.
        * @param final    Target value reached at the end of the duration.
        * @param duration Animation duration in seconds (must be > 0).
        * @param ttype    Interpolation curve to use (default: LINEAR).
        * @param callback Optional function invoked when the tween completes.
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
         * Interpolates the property and applies the new value through the setter.
         *
         * If the tween was created manually, call this from a Node::onPhysicsProcess()
         * function. Do not call it if the tween was created with Node::createPropertyTween().
         *
         * @param deltaTime Time elapsed since the previous update (seconds).
         * @return true when the tween has finished, false while it is running.
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
        /** Total animation duration (seconds). */
        float durationTime;
        /** Accumulated elapsed time since start (seconds). */
        float elapsedTime{0.0f};
        /** Target value reached at the end of the tween. */
        T targetValue;
        /** Starting value applied at time 0. */
        T startValue;
        /** Target object on which the setter is invoked every update. */
        Object* targetObject;
        /** Pointer to member function used to set the property value. */
        Setter setter;
    };

}