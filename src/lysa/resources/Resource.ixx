/*
 * Copyright (c) 2025-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
export module lysa.resources.resource;

import std;
import lysa.global;

export namespace lysa {

    /**
     * Base class for resources.
     */
    class Resource : public Object {
    public:

        Resource(const std::wstring& name);

        /**
         * Returns the unique id of the resource
         */
        auto getId() const { return id; }

        /**
         * Return the name of the resource
         */
        const auto& getName() const { return name; }

        bool operator==(const Resource &other) const { return id == other.id; }

        bool operator<(const Resource &other) const { return id < other.id; }

        bool operator>(const Resource &other) const { return id > other.id; }

        /**
         * Duplicates a resource. Warning: not implemented on all resource types, check documentation for the resource type before using it.
         */
        virtual std::shared_ptr<Resource> duplicate() const;

    protected:
        void setUpdated() { updated = framesInFlight; }

    private:
        unique_id          id;
        static unique_id   currentId;
        const std::wstring name;

        friend class Scene;
        uint32 updated{0};
        uint32 framesInFlight{0};

        auto isUpdated() const { return updated > 0;}
    };

}
