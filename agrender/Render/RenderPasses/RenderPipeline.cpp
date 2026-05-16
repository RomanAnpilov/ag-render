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
}

void RenderPipeline::render(FrameData frame_data)
{
    frame_data.command_allocator->reset();
    frame_data.command_buffer->beginCommandBuffer(frame_data.command_allocator);
        
    frame_data.render_pass_descriptor->depthAttachment()->setLoadAction(MTL::LoadActionClear);
    frame_data.render_pass_descriptor->depthAttachment()->setStoreAction(MTL::StoreActionDontCare);
    frame_data.render_pass_descriptor->depthAttachment()->setClearDepth(1.0);

    
    MTL4::RenderCommandEncoder* render_encoder = frame_data.command_buffer->renderCommandEncoder(frame_data.render_pass_descriptor);
//    setViewportSize(_viewportSize, pRenderEncoder); // maybe do not need it

    triangle_render_pass.draw(render_encoder);
    
    render_encoder->setDepthStencilState(depth_state);
    
    render_encoder->setCullMode(MTL::CullModeBack);
    render_encoder->setFrontFacingWinding(MTL::WindingCounterClockwise);
    
    render_encoder->drawPrimitives(MTL::PrimitiveTypeTriangle, 0, 3);

    render_encoder->endEncoding();
}
