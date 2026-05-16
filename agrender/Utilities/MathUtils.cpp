//
//  MathUtilities.cpp
//  agrender
//
//  Created by ANPILOV Roman on 21.02.2026.
//

#include "MathUtils.hpp"
#include <cmath>

simd_float4x4 MathUtils::Identity() {
    return matrix_identity_float4x4;
}

simd_float4x4 MathUtils::Perspective(float fovRadians, float aspect, float near, float far) {
    float ys = 1.0f / tanf(fovRadians * 0.5f);
    float xs = ys / aspect;
    float zs = far / (near - far);
    
    // Metal is Column-Major.
    // simd_matrix_from_rows constructs the matrix by rows,
    // which is often more intuitive for humans to write.
    return simd_matrix_from_rows(
        (simd_float4){ xs,  0,  0,           0 },
        (simd_float4){  0, ys,  0,           0 },
        (simd_float4){  0,  0, zs,  near * zs },
        (simd_float4){  0,  0, -1,           0 }
    );
}

simd_float4x4 MathUtils::LookAt(simd_float3 eye, simd_float3 target, simd_float3 up) {
    simd_float3 z = simd_normalize(eye - target);
    simd_float3 x = simd_normalize(simd_cross(up, z));
    simd_float3 y = simd_cross(z, x);
    
    return simd_matrix_from_rows(
        (simd_float4){ x.x, x.y, x.z, -simd_dot(x, eye) },
        (simd_float4){ y.x, y.y, y.z, -simd_dot(y, eye) },
        (simd_float4){ z.x, z.y, z.z, -simd_dot(z, eye) },
        (simd_float4){   0,   0,   0,                1 }
    );
}

float MathUtils::CalculateAspect(float width, float height) {
    return (height > 0) ? (width / height) : 1.0f;
}
