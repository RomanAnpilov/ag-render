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

typedef struct {
    simd_float4x4 viewProjectionMatrix;
    simd_float3   cameraPosition;
    simd_float4x4 modelViewMatrix;
} CameraUniforms;

// Material parameters for unlit rendering
typedef struct {
    simd_float4 baseColorFactor;
    simd_float3 emissiveFactor;
    int32_t baseColorTextureIndex;
    int32_t emissiveTextureIndex;
    uint32_t flags;  // Bit flags for texture presence
} MaterialParams;

// Material flags
#define MATERIAL_HAS_BASE_COLOR_TEXTURE  (1 << 0)
#define MATERIAL_HAS_EMISSIVE_TEXTURE    (1 << 1)

#endif /* ShaderTypes_h */
