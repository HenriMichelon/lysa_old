/*
 * Copyright (c) 2024-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module lysa.nodes.animation_player;

import lysa.global;

namespace lysa {

    void AnimationPlayer::seek(const float duration) {
        const auto animation = getAnimation();
        for (auto trackIndex = 0; trackIndex < animation->getTracksCount(); trackIndex++) {
            const auto& value = animation->getInterpolatedValue(
                       trackIndex,
                       duration,
                       false);
            currentTracksState[trackIndex] = value.frameTime;
            apply(value);
        }
    }

    void AnimationPlayer::apply(const Animation::TrackKeyValue& value) {
        switch (value.type) {
        case AnimationType::TRANSLATION:
            target->setPosition(value.value + initialPosition);
            break;
        case AnimationType::ROTATION:
            // target->setRotation(value.value + initialRotation); TODO
            break;
        case AnimationType::SCALE:
            // target->setScale(value.value + initialScale); TODO
            break;
        default:
            throw Exception("Unknown animation type");
        }
    }

    void AnimationPlayer::process(const float alpha) {
        Node::process(alpha);
        if (starting) {
            startTime = std::chrono::steady_clock::now();
            playing = true;
            starting = false;
            auto params = Playback{.animationName = currentAnimation};
            emit(on_playback_start, &params);
        } else if (!playing) {
            return;
        }
        const auto now = std::chrono::steady_clock::now();
        const auto duration = (std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime).count()) / 1000.0;
        const auto animation = getAnimation();
        if (animation && target) {
            for (auto trackIndex = 0; trackIndex < animation->getTracksCount(); trackIndex++) {
                const auto& value = animation->getInterpolatedValue(
                    trackIndex,
                    duration + lastTracksState[trackIndex],
                    reverse);
                currentTracksState[trackIndex] = value.frameTime;
                if (value.ended) {
                    stop();
                    auto params = Playback{.animationName = currentAnimation};
                    emit(on_playback_finish, &params);
                } else {
                   apply(value);
                }
            }
        }
    }

    void AnimationPlayer::enterScene() {
        Node::enterScene();
        if (!target && getParent()) {
            setTarget(getParent());
        }
        if (autoStart) {
            play();
        }
    }

    void AnimationPlayer::setTarget(Node *target) {
        this->target = target;
        initialPosition = target->getPosition();
        // initialRotation = target->getRotation(); TODO
        // initialScale = target->getScale(); TODO
    }


    void AnimationPlayer::setCurrentLibrary(const std::wstring &name) {
        if (libraries.contains(name)) {
            currentLibrary = name;
            setCurrentAnimation(libraries[currentLibrary]->getDefault());
        }
    }

    void AnimationPlayer::setCurrentAnimation(const std::wstring &name) {
        if (libraries[currentLibrary]->has(name)) {
            currentAnimation = name;
            if (currentTracksState.size() != getAnimation()->getTracksCount()) {
                currentTracksState.resize(getAnimation()->getTracksCount());
                lastTracksState.resize(getAnimation()->getTracksCount());
                std::ranges::fill(lastTracksState, 0.0f);
            }
        }
    }

    void AnimationPlayer::play(const std::wstring &name) {
        if (playing) { return; }
        if (name.empty()) {
            setCurrentAnimation(libraries[currentLibrary]->getDefault());
        } else {
            setCurrentAnimation(name);
        }
        starting = true;
        reverse = false;
    }

    void AnimationPlayer::playBackwards(const std::wstring &name) {
        if (playing) { return; }
        play(name);
        reverse = true;
    }

    void AnimationPlayer::stop(const bool keepState) {
        if (!playing) { return; }
        playing = false;
        if (keepState) {
            lastTracksState = currentTracksState;
        } else {
            std::ranges::fill(lastTracksState, 0.0f);
        }
    }

    std::shared_ptr<Animation> AnimationPlayer::getAnimation() {
        return libraries[currentLibrary]->get(currentAnimation);
    }

    std::shared_ptr<Node> AnimationPlayer::duplicateInstance() const {
        return std::make_shared<AnimationPlayer>(*this);
    }

}
