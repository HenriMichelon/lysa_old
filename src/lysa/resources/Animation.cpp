/*
 * Copyright (c) 2024-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module lysa.resources.animation;

import lysa.log;

namespace lysa {

    Animation::TrackKeyValue Animation::getInterpolatedValue(const uint32 trackIndex,
                                                             const double currentTimeFromStart,
                                                             const bool reverse) const {
        assert([&]{ return trackIndex < tracks.size(); }, "Track index out of range");
        const auto& track = tracks[trackIndex];
        auto value = TrackKeyValue{
            .ended = (!track.enabled ||
                    (loopMode == AnimationLoopMode::NONE && currentTimeFromStart >= track.duration) ||
                    track.keyTime.size() < 2),
            .type = track.type,
        };
        if (value.ended) {
            if (reverse) {
                value.value = track.keyValue[0];
            } else {
                value.value = track.keyValue[track.keyValue.size() - 1];
            }
            return value;
        }

        const auto currentTime = std::fmod(currentTimeFromStart, static_cast<double>(track.duration));
        value.frameTime = static_cast<float>(currentTime);

        const auto it = std::ranges::lower_bound(track.keyTime, static_cast<float>(currentTime));
        auto nextIndex = std::distance(track.keyTime.begin(), it);
        if (nextIndex == 0) {
            if (reverse) {
                value.value = track.keyValue[track.keyValue.size() - 1];
            } else {
                value.value = track.keyValue[0];
            }
            return value;
        }

        auto       previousIndex = nextIndex;
        const bool overflow = nextIndex == track.keyTime.size();

        if (reverse) {
            previousIndex = track.keyTime.size() - previousIndex;
            nextIndex = track.keyTime.size() - nextIndex;
        }

        const auto& previousTime = track.keyTime[previousIndex];
        const auto nextTime = overflow ? track.duration : track.keyTime[nextIndex];
        const auto diffTime = nextTime - previousTime;
        const auto interpolationValue = static_cast<float>((currentTime - previousTime) / (diffTime > 0 ? diffTime : 1.0f));

        const auto& previousValue = track.keyValue[previousIndex];
        if (track.interpolation == AnimationInterpolation::LINEAR) {
            const auto nextValue = overflow ? track.keyValue[0] : track.keyValue[nextIndex];
            switch (track.type) {
            case AnimationType::TRANSLATION:
            case AnimationType::SCALE:
                value.value = lerp(std::get<float3>(previousValue), std::get<float3>(nextValue), interpolationValue);
                break;
            case AnimationType::ROTATION:
                value.value = lerp(std::get<quaternion>(previousValue), std::get<quaternion>(nextValue), interpolationValue);
                break;
            }
        } else {
            // STEP
            value.value = previousValue;
        }
        return value;
    }

    Animation::Animation(const std::wstring &name): Resource{name} {}

    Animation::Animation(const uint32 tracksCount, const std::wstring &name): Resource {name} {
        tracks.resize(tracksCount);
    }

}
