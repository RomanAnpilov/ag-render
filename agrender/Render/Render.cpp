//
//  Render.cpp
//  agrender
//
//  Created by ANPILOV Roman on 05.02.2026.
//

#include "Render.hpp"
#include "ShaderTypes.h"
#include <cmath>

#define kMaxFramesInFlight 3

void Renderer::draw(MTK::View* pView)
{
    _frameNumber += 1;
    const uint32_t frameIndex = _frameNumber % kMaxFramesInFlight;
    if (_frameNumber > kMaxFramesInFlight) {
        waitOnSharedEvent(_pSharedEvent, _frameNumber - kMaxFramesInFlight);
    }
    
    FrameData frame_data = {
        .command_allocator = _commandAllocators[frameIndex],
        .command_buffer = command_buffer,
        .render_pass_descriptor = pView->currentMTL4RenderPassDescriptor(),
        .vertex_argument_table = _pVertexArgumentTable,
        .residency_set = _pResidencySet
    };
    
    update_render_uniforms();
    
    // TODO: Move somewhere to manager all gpu resources
    _pVertexArgumentTable->setAddress(gpu_model.vertexBuffer->gpuAddress(), 0);
    _pVertexArgumentTable->setAddress(render_uniforms_buffer->gpuAddress(), 1);
    
    render_pipeline.render(frame_data, gpu_model);
    
    command_buffer->endCommandBuffer();
    submitCommandBuffer(command_buffer, command_queue, pView);
    command_queue->signalEvent(_pSharedEvent, _frameNumber);
}

void Renderer::update_render_uniforms()
{
//    _mainCamera.SetPosition(_mainCamera._position + simd::float3{0.01});
    CGSize size = mtk_view->drawableSize();
    CameraUniforms camera_uniforms = _mainCamera.GetUpdateUniforms(static_cast<float>(size.width), static_cast<float>(size.height));
    uniforms.viewMatrix = camera_uniforms.viewMatrix;
    uniforms.projectionMatrix = camera_uniforms.projectionMatrix;
    uniforms.modelMatrix = MathUtils::Identity();
    std::memcpy(render_uniforms_buffer->contents(), &uniforms, sizeof(uniforms));
}

std::vector<MTL4::CommandAllocator*> Renderer::makeCommandAllocators(uint count) {
    std::vector<MTL4::CommandAllocator*> allocatorArray;
    allocatorArray.reserve(count);
    for (uint32_t allocatorNumber = 0; allocatorNumber < count; ++allocatorNumber) {
        MTL4::CommandAllocator* pAllocator = device->newCommandAllocator();
        if (pAllocator) {
            allocatorArray.push_back(pAllocator);
        }
    }
    return allocatorArray;
}

void Renderer::waitOnSharedEvent(MTL::SharedEvent* pSharedEvent, uint64_t earlierFrameNumber) {
    const uint64_t tenMilliseconds = 10;
    bool beforeTimeout = pSharedEvent->waitUntilSignaledValue(earlierFrameNumber, tenMilliseconds);
    if (!beforeTimeout) {
        std::cerr << "No signal from frame " << earlierFrameNumber << " to shared event after " << tenMilliseconds << "ms" << std::endl;
    }
}

Renderer::Renderer(MTL::Device* pDevice, MTK::View* pView):
device(pDevice->retain()),
mtk_view(pView->retain()),
_mainCamera(Camera())
{
    initializeCoreInstances();
    initializeResources();
    CGSize size = pView->drawableSize();
    updateViewportSize({static_cast<float>(size.width), static_cast<float>(size.height)});
    
    RenderPipelineInitializeData initialize_data = {
        .device = device,
        .color_pixel_format = pView->colorPixelFormat(),
        .depth_pixel_format = pView->depthStencilPixelFormat(),
        .view = mtk_view
    };
    
    render_pipeline.initialize(initialize_data);
}

void Renderer::initializeCoreInstances()
{
    command_queue = device->newMTL4CommandQueue();
    command_buffer = device->newCommandBuffer();
    _commandAllocators = makeCommandAllocators(kMaxFramesInFlight);
    _frameNumber = 0;
    _pSharedEvent = device->newSharedEvent();
    _pSharedEvent->setSignaledValue(_frameNumber);
}

void Renderer::initializeResources()
{
    NS::Error* pError = nullptr;
    
    // VertexArgumentTable
    MTL4::ArgumentTableDescriptor* vertexTableDescriptor = MTL4::ArgumentTableDescriptor::alloc()->init();
    vertexTableDescriptor->setMaxBufferBindCount(3);
    _pVertexArgumentTable = device->newArgumentTable(vertexTableDescriptor, &pError);
    vertexTableDescriptor->release();
    
    if (pError) { std::cerr << "Failed to create Vertex Argument Table: " << pError->localizedDescription()->utf8String() << std::endl; }
    
    // FragmentArgumentTable
    MTL4::ArgumentTableDescriptor* fragmentTableDescriptor = MTL4::ArgumentTableDescriptor::alloc()->init();
    fragmentTableDescriptor->setMaxBufferBindCount(3);
    _pFragmentArgumentTable = device->newArgumentTable(fragmentTableDescriptor, &pError);
    fragmentTableDescriptor->release();
    
    if (pError) { std::cerr << "Failed to create Fragment Argument Table: " << pError->localizedDescription()->utf8String() << std::endl; }
    
    // ResidencySet
    MTL::ResidencySetDescriptor* residencySetDescriptor = MTL::ResidencySetDescriptor::alloc()->init();
    _pResidencySet = device->newResidencySet(residencySetDescriptor, &pError);
    residencySetDescriptor->release();
    
    if (pError) { std::cerr << "Failed to create Residency Set: " << pError->localizedDescription()->utf8String() << std::endl; }
    
    // TODO: here add metalview residency set
//    _pCommandQueue->addResidencySet();
    
    AssetImporter importer;
    CpuModel model = importer.loadModel("/Users/ruaapr3/Developer/agrender/BoxTextured.gltf");
    
    GpuModelUploader uploader;
    uploader.initialize(device);
    gpu_model = uploader.upload(model);
    
    render_uniforms_buffer = device->newBuffer(sizeof(GpuUniforms), MTL::ResourceStorageModeShared);
    _pResidencySet->addAllocation(render_uniforms_buffer);

    _pResidencySet->addAllocation(gpu_model.indexBuffer);
    _pResidencySet->addAllocation(gpu_model.vertexBuffer);
    
    
    // Buffers, textures and etc
//    _triangleVertexBuffers = makeTriangleDataBuffers(kMaxFramesInFlight);
    _pViewportSizeBuffer = device->newBuffer(sizeof(_viewportSize), MTL::ResourceStorageModeShared);
    _pUniformsBuffer = device->newBuffer(sizeof(CameraUniforms), MTL::ResourceStorageModeShared);
//    uniforms.modelViewMatrix = MathUtils::Identity();
    
    // Add allocations to residency set
    _pResidencySet->addAllocation(_pViewportSizeBuffer);
    _pResidencySet->addAllocation(_pUniformsBuffer);
    
    for (auto buffer : _triangleVertexBuffers)
        _pResidencySet->addAllocation(buffer);
    
    _pResidencySet->commit();
    command_queue->addResidencySet(_pResidencySet);
}

void Renderer::updateViewportSize(const simd::float2& size) {
    _viewportSize.x = size.x;
    _viewportSize.y = size.y;
    void* pBufferContents = _pViewportSizeBuffer->contents();
    std::memcpy(pBufferContents, &_viewportSize, sizeof(_viewportSize));
}

void Renderer::setViewportSize(simd_uint2 size, MTL4::RenderCommandEncoder* pRenderEncoder) {
    MTL::Viewport viewport;
    viewport.originX = 0.0;
    viewport.originY = 0.0;
    viewport.znear   = 0.0;
    viewport.zfar    = 1.0;
    viewport.width   = static_cast<double>(size.x);
    viewport.height  = static_cast<double>(size.y);
    pRenderEncoder->setViewport(viewport);
}

void Renderer::submitCommandBuffer(MTL4::CommandBuffer* pCommandBuffer,
                                   MTL4::CommandQueue* pCommandQueue,
                                   MTK::View* pView)
{
    CA::MetalDrawable* pCurrentDrawable = pView->currentDrawable();
    if (!pCurrentDrawable) {
        return;
    }
    pCommandQueue->wait(pCurrentDrawable);
    MTL4::CommandBuffer* commandBuffers[] = { pCommandBuffer };
    pCommandQueue->commit(commandBuffers, 1);
    pCommandQueue->signalDrawable(pCurrentDrawable);
    pCurrentDrawable->present();
}

Renderer::~Renderer()
{
    command_buffer->release();
    command_queue->release();
    device->release();
}
