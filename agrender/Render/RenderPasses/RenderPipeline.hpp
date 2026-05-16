#pragma once

#include <Metal/Metal.hpp>
#include "TriangleRenderPass.hpp"
#include "GpuUploader.hpp"

struct RenderPipelineInitializeData
{
    MTL::Device* device;
    MTL::PixelFormat color_pixel_format;
    MTL::PixelFormat depth_pixel_format;
};

struct FrameData
{
    MTL4::CommandAllocator* command_allocator;
    MTL4::CommandBuffer* command_buffer;
    MTL4::RenderPassDescriptor* render_pass_descriptor;
    MTL4::ArgumentTable* vertex_argument_table;
};

class RenderPipeline {
public:
    void initialize(RenderPipelineInitializeData initalize_data);
    void render(FrameData frame_data, const GpuModel& gpu_model);
private:
    // TODO: Add support for opaque, mask and blend
    MTL::DepthStencilState* depth_state;
    
    TriangleRenderPass triangle_render_pass;
private:
    MTL::Device* device;
    MTL::Library* shader_library;
    MTL::PixelFormat color_pixel_format;
    MTL::PixelFormat depth_pixel_format;
};
