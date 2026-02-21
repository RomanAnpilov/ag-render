//
//  Shaders.metal
//  agrender
//
//  Created by ANPILOV Roman on 05.02.2026.
//

#include <metal_stdlib>
using namespace metal;

#include "ShaderTypes.h"

struct RasterizerData
{
    float4 position [[position]];
    float4 color;
};

//InputBufferIndexForVertexData
//InputBufferIndexForViewportSize
vertex RasterizerData
vertexShader(uint vertexID [[vertex_id]],
             constant VertexData *vertexData [[buffer(0)]],
             constant simd_uint2 *viewportSizePointer [[buffer(1)]],
             constant Uniforms *uniforms [[buffer(2)]])
{
    RasterizerData out;
    
    VertexData v = vertexData[vertexID];
    Uniforms u = *(uniforms);
    
    float4 pos = float4(v.position, 1.0);
    
    // Perform 3D transformation
    out.position = u.projectionMatrix * u.modelViewMatrix * pos;
    out.color = v.color;
    
    return out;
}

/// A basic fragment shader that returns the color data from the rasterizer
/// without modifying it.
fragment float4 fragmentShader(RasterizerData in [[stage_in]])
{
    // Return the color the rasterizer interpolates between the triangle's
    // three vertex colors.
    return in.color;
}
