//
//  TriangleRenderPass.hpp
//  agrender
//
//  Created by ANPILOV Roman on 16.05.2026.
//

#include <Metal/Metal.hpp>
#include "GpuUploader.hpp"

class TriangleRenderPass
{
public:
    void initialize(MTL::Device* device, MTL::Library* shader_library, MTL::PixelFormat colorPixelFormat);
    void draw(MTL4::RenderCommandEncoder* render_encoder, const GpuModel& gpu_model);
private:
    MTL::RenderPipelineState* compileRenderPipeline(MTL::PixelFormat colorPixelFormat);
private:
    MTL::RenderPipelineState* render_pso;
    
    MTL::Device* device;
    MTL::Library* shader_library;
};
