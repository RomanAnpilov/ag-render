//
//  Render.hpp
//  agrender
//
//  Created by ANPILOV Roman on 05.02.2026.
//

#include <Metal/Metal.hpp>
#include <AppKit/AppKit.hpp>
#include <MetalKit/MetalKit.hpp>
#include <vector>
#include <chrono>
#include <iostream>
#include "TriangleData.h"

class Renderer
{
public:
    Renderer(MTL::Device* pDevice, MTK::View* pView);
    ~Renderer();
    void draw(MTK::View* pView);
    
private:
    void updateViewportSize(const simd::float2& size);
    
    std::vector<MTL::Buffer*> makeTriangleDataBuffers(uint count);
    MTL4::ArgumentTable* makeArgumentTable();
    MTL::ResidencySet* makeResidencySet();
    std::vector<MTL4::CommandAllocator*> makeCommandAllocators(uint count);
    MTL::RenderPipelineState* compileRenderPipeline(MTL::PixelFormat colorPixelFormat);
    MTL4::Compiler* createDefaultMetalCompiler();
    void waitOnSharedEvent(MTL::SharedEvent* pSharedEvent, uint64_t earlierFrameNumber);
    void setViewportSize(simd_uint2 size, MTL4::RenderCommandEncoder* pRenderEncoder);
    void setRenderPassArguments(MTL4::RenderCommandEncoder* pRenderEncoder,
                                NS::UInteger frameNumber,
                                MTL4::ArgumentTable* pArgumentTable,
                                MTL::Buffer* pVertexBuffer,
                                MTL::Buffer* pViewportSizeBuffer);
    
    void submitCommandBuffer(MTL4::CommandBuffer* pCommandBuffer,
                             MTL4::CommandQueue* pCommandQueue,
                             MTK::View* pView);

    
    MTL::Device* _pDevice;
    MTL4::CommandQueue* _pCommandQueue;
    MTL4::CommandBuffer* _pCommandBuffer;
    MTL::Library* _pDefaultLibrary;
    MTL4::ArgumentTable* _pArgumentTable;
    MTL::ResidencySet* _pResidencySet;
    MTL::SharedEvent* _pSharedEvent;
    MTL::RenderPipelineState* _pRenderPipelineState;
    MTL::DepthStencilState* pDepthState;
    
    uint64_t _frameNumber;
    simd_uint2 _viewportSize;
    Uniforms uniforms;
    
    MTL::Buffer* _pViewportSizeBuffer;
    MTL::Buffer* _pUniformsBuffer;
    std::vector<MTL::Buffer *> _triangleVertexBuffers;
    std::vector<MTL4::CommandAllocator*> _commandAllocators;
    
};
