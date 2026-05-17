//
//  Camera.hpp
//  agrender
//
//  Created by ANPILOV Roman on 21.02.2026.
//

#ifndef CAMERA_HPP
#define CAMERA_HPP

#include "MathUtils.hpp"
#include "ShaderTypes.h"

class Camera {
public:
    Camera();

    void SetPosition(simd_float3 pos);
    void SetTarget(simd_float3 target);
    void SetFOV(float degrees);

    CameraUniforms GetUpdateUniforms(float screenWidth, float screenHeight);
    
    simd_float3 _position;
private:
    simd_float3 _target;
    simd_float3 _up;
    float       _fovRadians;
    float       _near;
    float       _far;
};

#endif
