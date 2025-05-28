/*
 * Copyright (c) 2024-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
export module lysa.resources.animation_library;

import lysa.resources.animation;
import lysa.resources.resource;

export namespace lysa {

    /**
     * Container for \ref Animation resources.
     */
    class AnimationLibrary : public Resource {
    public:
        /**
         * Creates an AnimationLibrary
         * @param name resource name.
         */
        AnimationLibrary(const std::wstring &name = L"AnimationLibrary") : Resource{name} {}

        /**
         * Adds the \ref Animation to the library, accessible by the key name.
         */
        void add(const std::wstring& keyName, const std::shared_ptr<Animation>& animation) {
            if (animations.empty()) {
                defaultAnimation = keyName;
            }
            animations[keyName] = animation;
        }

        /**
         * Returns the \ref Animation with the key name.
         */
        auto get(const std::wstring& keyName) const { return animations.at(keyName); }

        /**
         * Returns `true` if the library stores an \ref Animation with name as the key.
         */
        auto has(const std::wstring& keyName) const { return animations.contains(keyName); }

        /**
         * Returns the name of the default animation
         */
        const auto& getDefault() const { return defaultAnimation; }

    private:
        std::wstring defaultAnimation;
        std::map<std::wstring, std::shared_ptr<Animation>> animations;
    };

}
