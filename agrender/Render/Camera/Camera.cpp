//
//  Camera.cpp
//  agrender
//
//  Created by ANPILOV Roman on 21.02.2026.
//

#include "Camera.hpp"
#include <cmath>

Camera::Camera() {
    _position = (simd_float3){0.0f, 0.0f, 5.0f};
    _target   = (simd_float3){0.0f, 0.0f, 0.0f};
    _up       = (simd_float3){0.0f, 1.0f, 0.0f};
    _fovRadians = 45.0f * (M_PI / 180.0f);
    _near = 0.1f;
    _far  = 1000.0f;
}

void Camera::SetPosition(simd_float3 pos) {
    _position = pos;
}

void Camera::SetTarget(simd_float3 target) {
    _target = target;
}

void Camera::SetFOV(float degrees) {
    _fovRadians = degrees * (M_PI / 180.0f);
}

CameraUniforms Camera::GetUpdateUniforms(float width, float height) {
    float aspect = MathUtils::CalculateAspect(width, height);
    
    simd_float4x4 viewMatrix = MathUtils::LookAt(_position, _target, _up);
    simd_float4x4 projectionMatrix = MathUtils::Perspective(_fovRadians, aspect, _near, _far);
    
    CameraUniforms uniforms;
    
    uniforms.viewProjectionMatrix = simd_mul(projectionMatrix, viewMatrix);
    
    uniforms.cameraPosition = _position;
    
    return uniforms;
}
