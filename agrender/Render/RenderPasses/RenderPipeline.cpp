//
//  RenderPipeline.cpp
//  agrender
//
//  Created by ANPILOV Roman on 16.05.2026.
//

#include "RenderPipeline.hpp"

void RenderPipeline::initialize(RenderPipelineInitializeData initalize_data)
{
    device = initalize_data.device->retain();
    shader_library = device->newDefaultLibrary();
    color_pixel_format = initalize_data.color_pixel_format;
    depth_pixel_format = initalize_data.depth_pixel_format;
    
    MTL::DepthStencilDescriptor* depth_descriptor = MTL::DepthStencilDescriptor::alloc()->init();
    depth_descriptor->setDepthCompareFunction(MTL::CompareFunctionLess);
    depth_descriptor->setDepthWriteEnabled(true);

    depth_state = device->newDepthStencilState(depth_descriptor);
    
    depth_descriptor->release();
    
    triangle_render_pass.initialize(device, shader_library, color_pixel_format);
    imgui_render_pass.initialize(device, initalize_data.view);
}

void RenderPipeline::render(FrameData frame_data, const GpuModel& gpu_model)
{
    frame_data.command_allocator->reset();
    frame_data.command_buffer->beginCommandBuffer(frame_data.command_allocator);
        
    frame_data.render_pass_descriptor->depthAttachment()->setLoadAction(MTL::LoadActionClear);
    frame_data.render_pass_descriptor->depthAttachment()->setStoreAction(MTL::StoreActionDontCare);
    frame_data.render_pass_descriptor->depthAttachment()->setClearDepth(1.0);

    
    MTL4::RenderCommandEncoder* render_encoder = frame_data.command_buffer->renderCommandEncoder(frame_data.render_pass_descriptor);
//    setViewportSize(_viewportSize, pRenderEncoder); // maybe do not need it
    render_encoder->setArgumentTable(frame_data.vertex_argument_table, MTL::RenderStageVertex);
    render_encoder->setDepthStencilState(depth_state);
    render_encoder->setCullMode(MTL::CullModeBack);
    render_encoder->setFrontFacingWinding(MTL::WindingCounterClockwise);
    
    triangle_render_pass.draw(render_encoder, gpu_model);
    imgui_render_pass.draw(frame_data.render_pass_descriptor, frame_data.command_buffer, render_encoder, frame_data.residency_set);
    
    render_encoder->endEncoding();
}
