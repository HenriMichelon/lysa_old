/*
* Copyright (c) 2025-present Henri Michelon
 *
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module lysa.resources.resource;

import lysa.global;

namespace lysa {

    id_t Resource::currentId{INVALID_ID};

    Resource::Resource(const std::wstring& name):
        id{++currentId},
        name{sanitizeName(name)} {
    }

    std::shared_ptr<Resource> Resource::duplicate() const {
        throw Exception{"Not implemented"};
    }


}
