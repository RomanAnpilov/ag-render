////
////  Shaders.metal
////  agrender
////
////  Created by ANPILOV Roman on 05.02.2026.
////
//
//#include <metal_stdlib>
//using namespace metal;
//
//#include "ShaderTypes.h"
//
//struct RasterizerData
//{
//    float4 position [[position]];
//    float4 color;
//};
//
////InputBufferIndexForVertexData
////InputBufferIndexForViewportSize
////vertex RasterizerData
////vertexShader(uint vertexID [[vertex_id]],
////             constant VertexData *vertexData [[buffer(0)]],
////             constant simd_uint2 *viewportSizePointer [[buffer(1)]],
////             constant CameraUniforms *uniforms [[buffer(2)]])
////{
////    RasterizerData out;
////    
////    VertexData v = vertexData[vertexID];
////    CameraUniforms u = *(uniforms);
////    
////    float4 pos = float4(v.position, 1.0);
////    
////    // Perform 3D transformation
////    out.position = u.viewProjectionMatrix * u.modelViewMatrix * pos;
////    out.color = v.color;
////    
////    return out;
////}
////
/////// A basic fragment shader that returns the color data from the rasterizer
/////// without modifying it.
////fragment float4 fragmentShader(RasterizerData in [[stage_in]])
////{
////    // Return the color the rasterizer interpolates between the triangle's
////    // three vertex colors.
////    return in.color;
////}
//
//#include <metal_stdlib>
//using namespace metal;
//
//struct Vertex {
//    float3 position;
//    float3 normal;
//    float2 uv;
//};
//
//struct PBRFactors {
//    float4 baseColorFactor;
//    float metallicFactor;
//    float roughnessFactor;
//};
//
//struct VertexOut {
//    float4 position [[position]];
//    float2 uv;
//    float3 normal;
//};
//
//// VERTEX SHADER
//// vertexArray is pulled from the Vertex Argument Table at index 0
//vertex VertexOut vertexMain(uint vertexID [[vertex_id]],
//                            device const Vertex* vertexArray [[buffer(0)]],
//                            constant CameraUniforms *uniforms [[buffer(2)]])
//{
//    VertexOut out;
//    device const Vertex& v = vertexArray[vertexID];
//    CameraUniforms u = *(uniforms);
//
//    // Remember to multiply by an MVP matrix in a real scenario
//    out.position = u.viewProjectionMatrix * u.modelViewMatrix * float4(v.position, 1.0);
//    out.uv = v.uv;
//    out.normal = v.normal;
//    return out;
//}
//
//// FRAGMENT SHADER
//// factors is pulled from Fragment Argument Table at buffer index 0
//// baseColorTex is pulled from Fragment Argument Table at texture index 0
//fragment float4 fragmentMain(VertexOut in [[stage_in]],
//                             device const PBRFactors* factors [[buffer(0)]],
//                             texture2d<float> baseColorTex [[texture(0)]])
//{
//    sampler texSampler(mag_filter::linear, min_filter::linear);
//    
//    // Sample texture and multiply by glTF base color factor
//    float4 color = baseColorTex.sample(texSampler, in.uv) * factors->baseColorFactor;
//    
//    // Standard lighting calculation here...
//    float3 lightDir = normalize(float3(1.0, 1.0, 1.0));
//    float diff = max(dot(normalize(in.normal), lightDir), 0.1);
//    
//    return float4(color.rgb * diff, color.a);
//}


#include <metal_stdlib>
using namespace metal;

struct Vertex {
    float3 position;
    float2 uv;
    float3 normal;
    float4 color;
};

struct VSOut {
    float4 position [[position]];
};

vertex VSOut vertexMain(uint vid [[vertex_id]], device const Vertex* verticies [[buffer(0)]])
{
    Vertex v = verticies[vid];

    VSOut out;
    out.position = float4(v.position, 1.0);
    return out;
}

fragment half4 fragmentMain()
{
    return half4(1.0, 0.2, 0.1, 1.0);
}
