/*
* Copyright (c) 2025-present Henri Michelon
 *
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
export module lysa.constants;

#include <Eigen/Dense>
using namespace Eigen;

export namespace lysa {
    /**
    * Default clear color for windows and color frame buffers
    */
    const Vector3f DEFAULT_CLEAR_COLOR{0.0f, 0.0f, 0.0f};

    /**
    * X Axis
    */
    const Vector3f AXIS_X{1.0, 0.0f, 0.0f};

    /**
    * Y Axis
    */
    const Vector3f AXIS_Y{0.0, 1.0f, 0.0f};

    /**
    * Z Axis
    */
    const Vector3f AXIS_Z{0.0, 0.0f, 1.0f};

    /**
    * UP Axis
    */
    const Vector3f AXIS_UP = AXIS_Y;

    /**
    * DOWN Axis
    */
    const Vector3f AXIS_DOWN = -AXIS_Y;

    /**
    * FRONT Axis
    */
    const Vector3f AXIS_FRONT = -AXIS_Z;

    /**
    * BACK Axis
    */
    const Vector3f AXIS_BACK = AXIS_Z;

    /**
    * RIGHT Axis
    */
    const Vector3f AXIS_RIGHT = AXIS_X;

    /**
    * LEFT Axis
    */
    const Vector3f AXIS_LEFT = -AXIS_X;

    /**
    * 2D zero initialized vector
    */
    const Vector2f VEC2ZERO{0.0};

    /**
    * 3D zero initialized vector
    */
    const Vector3f VEC3ZERO{0.0};

    /**
    * Unit quaternion with no rotation
    */
    const Quaternion QUATERNION_IDENTITY{1.0f, 0.0f, 0.0f, 0.0f};

    /**
    * The Basis of 3D transform.
    * It is composed of 3 axes (Basis.x, Basis.y, and Basis.z).
    * Together, these represent the transform's rotation, scale, and shear.
    */
    constexpr glm::mat3 TRANSFORM_BASIS{1, 0, 0, 0, 1, 0, 0, 0, 1};
}
