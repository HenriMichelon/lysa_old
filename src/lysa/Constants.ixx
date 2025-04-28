/*
* Copyright (c) 2025-present Henri Michelon
 *
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
export module lysa.constants;

import glm;

export namespace lysa {
    /**
    * Default clear color for windows and color frame buffers
    */
    constexpr glm::vec3 DEFAULT_CLEAR_COLOR{0.0f, 0.0f, 0.0f};

    /**
    * X Axis
    */
    constexpr glm::vec3 AXIS_X{1.0, 0.0f, 0.0f};

    /**
    * Y Axis
    */
    constexpr glm::vec3 AXIS_Y{0.0, 1.0f, 0.0f};

    /**
    * Z Axis
    */
    constexpr glm::vec3 AXIS_Z{0.0, 0.0f, 1.0f};

    /**
    * UP Axis
    */
    constexpr glm::vec3 AXIS_UP = AXIS_Y;

    /**
    * DOWN Axis
    */
    constexpr glm::vec3 AXIS_DOWN = -AXIS_Y;

    /**
    * FRONT Axis
    */
    constexpr glm::vec3 AXIS_FRONT = -AXIS_Z;

    /**
    * BACK Axis
    */
    constexpr glm::vec3 AXIS_BACK = AXIS_Z;

    /**
    * RIGHT Axis
    */
    constexpr glm::vec3 AXIS_RIGHT = AXIS_X;

    /**
    * LEFT Axis
    */
    constexpr glm::vec3 AXIS_LEFT = -AXIS_X;

    /**
    * 2D zero initialized vector
    */
    constexpr glm::vec2 VEC2ZERO{0.0};

    /**
    * 3D zero initialized vector
    */
    constexpr glm::vec3 VEC3ZERO{0.0};

    /**
    * Unit quaternion with no rotation
    */
    constexpr glm::quat QUATERNION_IDENTITY{1.0f, 0.0f, 0.0f, 0.0f};

    /**
    * The Basis of 3D transform.
    * It is composed of 3 axes (Basis.x, Basis.y, and Basis.z).
    * Together, these represent the transform's rotation, scale, and shear.
    */
    constexpr glm::mat3 TRANSFORM_BASIS{1, 0, 0, 0, 1, 0, 0, 0, 1};
}
