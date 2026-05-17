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
#include <memory>

#include "MathUtils.hpp"
#include "Camera.hpp"
#include "RenderPipeline.hpp"
#include "AssetImporter.hpp"
#include "GpuUploader.hpp"

class Renderer
{
public:
    Renderer(MTL::Device* pDevice, MTK::View* pView);
    ~Renderer();
    void draw(MTK::View* pView);
    
private:
    GpuUniforms uniforms;
    MTL::Buffer* render_uniforms_buffer;
    void update_render_uniforms();
    
    GpuModel gpu_model;
    MTL::Device* device;
    MTK::View* mtk_view;
    MTL4::CommandQueue* command_queue;
    MTL4::CommandBuffer* command_buffer;
    std::vector<MTL4::CommandAllocator*> _commandAllocators;
    std::vector<MTL4::CommandAllocator*> makeCommandAllocators(uint count);
    void initializeCoreInstances();
    RenderPipeline render_pipeline;
    // Manage all metal-core instances
    
    // Resources, buffers and etc
    MTL4::ArgumentTable* _pVertexArgumentTable;
    MTL4::ArgumentTable* _pFragmentArgumentTable;
    MTL::ResidencySet* _pResidencySet;
    MTL::Buffer* _pViewportSizeBuffer;
    MTL::Buffer* _pUniformsBuffer;
    std::vector<MTL::Buffer *> _triangleVertexBuffers;
    void initializeResources();
    
    // Synchronizations
    uint64_t _frameNumber;
    MTL::SharedEvent* _pSharedEvent;
    void waitOnSharedEvent(MTL::SharedEvent* pSharedEvent, uint64_t earlierFrameNumber);
    
    // Utilites and services
    Camera _mainCamera;
    simd_uint2 _viewportSize;
//    CameraUniforms uniforms;
    void updateViewportSize(const simd::float2& size);
    void setViewportSize(simd_uint2 size, MTL4::RenderCommandEncoder* pRenderEncoder);

    void submitCommandBuffer(MTL4::CommandBuffer* pCommandBuffer,
                             MTL4::CommandQueue* pCommandQueue,
                             MTK::View* pView);
};
