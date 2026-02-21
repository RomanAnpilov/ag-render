//
//  ShaderTypes.h
//  agrender
//
//  Created by ANPILOV Roman on 05.02.2026.
//

#ifndef ShaderTypes_h
#define ShaderTypes_h

#include <simd/simd.h>

typedef struct {
    simd_float3 position;
    simd_float4 color;
} VertexData;

typedef struct {
    simd_float4x4 projectionMatrix;
    simd_float4x4 modelViewMatrix;
} Uniforms;

#endif /* ShaderTypes_h */
