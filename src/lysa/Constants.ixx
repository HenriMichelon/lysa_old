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
    const float3 AXIS_X{1.0f, 0.0f, 0.0f};

    /**
    * Y Axis
    */
    const float3 AXIS_Y{0.0f, 1.0f, 0.0f};

    /**
    * Z Axis
    */
    const float3 AXIS_Z{0.0f, 0.0f, 1.0f};

    /**
    * UP Axis
    */
    const float3 AXIS_UP = {0.0f, 1.0f, 0.0f};

    /**
    * DOWN Axis
    */
    const float3 AXIS_DOWN = {0.0f, -1.0f, 0.0f};

    /**
    * FRONT Axis
    */
    const float3 AXIS_FRONT = {0.0f, 0.0f, -1.0f};

    /**
    * BACK Axis
    */
    const float3 AXIS_BACK = {0.0f, 0.0f, 1.0f};

    /**
    * RIGHT Axis
    */
    const float3 AXIS_RIGHT = {1.0f, 0.0f, 0.0f};

    /**
    * LEFT Axis
    */
    const float3 AXIS_LEFT = {-1.0f, 0.0f, 0.0f};

    /**
    * 2D zero initialized vector
    */
    const float2 FLOAT2ZERO{0.0f};

    /**
    * 3D zero initialized vector
    */
    const float3 FLOAT3ZERO{0.0f};

    /**
    * Unit quaternion with no rotation
    */
    const quaternion QUATERNION_IDENTITY{1.0f, 0.0f, 0.0f, 0.0f};

    /**
    * The Basis of 3D transform.
    * It is composed of 3 axes (Basis.x, Basis.y, and Basis.z).
    * Together, these represent the transform's rotation, scale, and shear.
    */
    const float3x3 TRANSFORM_BASIS{1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f};

    constexpr float HALF_PI = std::numbers::pi_v<float> / 2.0f;

    /**
     * Maximum number of parameters of a ShaderMaterial
     */
    constexpr int SHADER_MATERIAL_MAX_PARAMETERS{4};

    constexpr pipeline_id DEFAULT_PIPELINE_ID{0};

    constexpr uint32 MAX_COLLISIONS_LAYERS{100};


}
