/*
* Copyright (c) 2025-present Henri Michelon
 *
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
export module lysa.constants;

import std;
import lysa.math;
import lysa.types;

export namespace lysa {

    constexpr unique_id INVALID_ID{0};

    /**
    * Default clear color for windows and color frame buffers
    */
    const float3 DEFAULT_CLEAR_COLOR{0.0f, 0.0f, 0.0f};

    /**
    * X Axis
    */
    const float3 AXIS_X{1.0, 0.0f, 0.0f};

    /**
    * Y Axis
    */
    const float3 AXIS_Y{0.0, 1.0f, 0.0f};

    /**
    * Z Axis
    */
    const float3 AXIS_Z{0.0, 0.0f, 1.0f};

    /**
    * UP Axis
    */
    const float3 AXIS_UP = AXIS_Y;

    /**
    * DOWN Axis
    */
    const float3 AXIS_DOWN = -AXIS_Y;

    /**
    * FRONT Axis
    */
    const float3 AXIS_FRONT = -AXIS_Z;

    /**
    * BACK Axis
    */
    const float3 AXIS_BACK = AXIS_Z;

    /**
    * RIGHT Axis
    */
    const float3 AXIS_RIGHT = AXIS_X;

    /**
    * LEFT Axis
    */
    const float3 AXIS_LEFT = -AXIS_X;

    /**
    * 2D zero initialized vector
    */
    const float2 FLOAT2ZERO{0.0};

    /**
    * 3D zero initialized vector
    */
    const float3 FLOAT3ZERO{0.0};

    /**
    * Unit quaternion with no rotation
    */
    const quaternion QUATERNION_IDENTITY{1.0f, 0.0f, 0.0f, 0.0f};

    /**
    * The Basis of 3D transform.
    * It is composed of 3 axes (Basis.x, Basis.y, and Basis.z).
    * Together, these represent the transform's rotation, scale, and shear.
    */
    const float3x3 TRANSFORM_BASIS{1, 0, 0, 0, 1, 0, 0, 0, 1};

    constexpr float HALF_PI = std::numbers::pi_v<float> / 2.0f;

    /**
     * Maximum number of parameters of a ShaderMaterial
     */
    constexpr int SHADER_MATERIAL_MAX_PARAMETERS{4};

    constexpr pipeline_id DEFAULT_PIPELINE_ID{0};


}
