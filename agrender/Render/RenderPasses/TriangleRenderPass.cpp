//
//  TriangleRenderPass.cpp
//  agrender
//
//  Created by ANPILOV Roman on 16.05.2026.
//

#include "TriangleRenderPass.hpp"

void TriangleRenderPass::initialize(MTL::Device* device, MTL::Library* shader_library, MTL::PixelFormat colorPixelFormat)
{
    this->device = device;
    this->shader_library = shader_library;
    render_pso = compileRenderPipeline(colorPixelFormat);
}

void TriangleRenderPass::draw(MTL4::RenderCommandEncoder* render_encoder, const GpuModel& gpu_model)
{
    // put it somewhere
    
    //
    //    uniforms = _mainCamera.GetUpdateUniforms(1024, 1024);
    //    uniforms.modelViewMatrix = MathUtils::Identity();
    //    uniforms.modelViewMatrix = simd_mul(uniforms.modelViewMatrix, rotationY);
    //    void* pBufferContents = _pUniformsBuffer->contents();
    //    std::memcpy(pBufferContents, &uniforms, sizeof(uniforms));
    //
    ////    configureVertexDataForBufferCube(frameNumber, pVertexBuffer->contents());
    //    pArgumentTable->setAddress(pVertexBuffer->gpuAddress(), 0);
    //    pArgumentTable->setAddress(pViewportSizeBuffer->gpuAddress(), 1);
    //    pArgumentTable->setAddress(_pUniformsBuffer->gpuAddress(), 2);
    
    render_encoder->setRenderPipelineState(render_pso);
    
    for (const GpuMesh& gpu_mesh : gpu_model.meshses)
    {
        for (const GpuPrimitiveRange& primitive: gpu_mesh.primitives)
        {
            render_encoder->drawIndexedPrimitives(MTL::PrimitiveTypeTriangle,
                                                  primitive.index_count,
                                                  MTL::IndexTypeUInt32,
                                                  gpu_model.indexBuffer->gpuAddress(),
                                                  primitive.index_offset * sizeof(uint32_t));
        }
    }
    
//    render_encoder->setRenderPipelineState(render_pso);
//    render_encoder->drawPrimitives(MTL::PrimitiveTypeTriangle, 0, 3);
}

MTL::RenderPipelineState* TriangleRenderPass::compileRenderPipeline(MTL::PixelFormat colorPixelFormat) {
    NS::Error* pError = nullptr;

    MTL::RenderPipelineDescriptor* pDesc = MTL::RenderPipelineDescriptor::alloc()->init();
    pDesc->setLabel(NS::String::string("Basic Metal render pipeline", NS::UTF8StringEncoding));
    pDesc->colorAttachments()->object(0)->setPixelFormat(colorPixelFormat);
    pDesc->setDepthAttachmentPixelFormat(MTL::PixelFormatDepth32Float);
    
    MTL::Function* pVertexFunc = shader_library->newFunction(NS::String::string("vertexMain", NS::UTF8StringEncoding));
    MTL::Function* pFragmentFunc = shader_library->newFunction(NS::String::string("fragmentMain", NS::UTF8StringEncoding));

    // 3. Configure the descriptor
    pDesc->setVertexFunction(pVertexFunc);
    pDesc->setFragmentFunction(pFragmentFunc);

    // 4. Compile the state
    MTL::RenderPipelineState* pPipelineState = device->newRenderPipelineState(pDesc, &pError);

    if (!pPipelineState) {
        if (pError) {
            printf("Error compiling pipeline state: %s\n", pError->localizedDescription()->utf8String());
        }
        assert(pPipelineState != nullptr);
    }

    pFragmentFunc->release();
    pVertexFunc->release();
    pDesc->release();

    return pPipelineState;
}
