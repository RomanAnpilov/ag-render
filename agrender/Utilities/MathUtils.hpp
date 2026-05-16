//
//  MathUtils.hpp
//  agrender
//
//  Created by ANPILOV Roman on 21.02.2026.
//

#ifndef MATH_UTILS_HPP
#define MATH_UTILS_HPP

#include <simd/simd.h>
#include "ShaderTypes.h"

struct MathUtils {
    static simd_float4x4 Identity();
    static simd_float4x4 Perspective(float fovRadians, float aspect, float near, float far);
    static simd_float4x4 LookAt(simd_float3 eye, simd_float3 target, simd_float3 up);
    static float CalculateAspect(float width, float height);
};


#endif
