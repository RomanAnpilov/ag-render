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

void configureVertexDataForBuffer(long rotationInDegrees, void *bufferContents);
void configureVertexDataForBufferCube(long frameNumber, void *bufferContents);

Renderer::Renderer(MTL::Device* pDevice, MTK::View* pView)
: _pDevice(pDevice->retain())
{
    _pCommandQueue = _pDevice->newMTL4CommandQueue();
    _pCommandBuffer = _pDevice->newCommandBuffer();
    _pDefaultLibrary = _pDevice->newDefaultLibrary();
    
    _triangleVertexBuffers = makeTriangleDataBuffers(kMaxFramesInFlight);
    _pViewportSizeBuffer = _pDevice->newBuffer(sizeof(_viewportSize), MTL::ResourceStorageModeShared);
    _pUniformsBuffer = _pDevice->newBuffer(sizeof(Uniforms), MTL::ResourceStorageModeShared);
    _pArgumentTable = makeArgumentTable();
    _pResidencySet = makeResidencySet();
    _commandAllocators = makeCommandAllocators(kMaxFramesInFlight);
    _pRenderPipelineState = compileRenderPipeline(pView->colorPixelFormat());
    
    MTL::DepthStencilDescriptor* pDepthDesc = MTL::DepthStencilDescriptor::alloc()->init();
    pDepthDesc->setDepthCompareFunction(MTL::CompareFunctionLess);
    pDepthDesc->setDepthWriteEnabled(true);

    pDepthState = pDevice->newDepthStencilState(pDepthDesc);
    pDepthDesc->release();
    
    _frameNumber = 0;
    
    _pSharedEvent = _pDevice->newSharedEvent();
    _pSharedEvent->setSignaledValue(_frameNumber);
    
    _pResidencySet->addAllocation(_pViewportSizeBuffer);
    _pResidencySet->addAllocation(_pUniformsBuffer);
    
    for (auto buffer : _triangleVertexBuffers)
        _pResidencySet->addAllocation(buffer);
    
    _pResidencySet->commit();
    _pCommandQueue->addResidencySet(_pResidencySet);
    //    _pCommandQueue->addResidencySet() here add metalview residency set
    
    CGSize size = pView->drawableSize();
    updateViewportSize({static_cast<float>(size.width), static_cast<float>(size.height)});
    
    
    float fovRadians = 45.0f * (M_PI / 180.0f);
    float aspect = (float)size.width / (float)size.height;
    float far = 100.0f;
    float near = 0.1f;

    float yScale = 1.0f / tanf(fovRadians * 0.5f);
    float xScale = yScale / aspect;
    float zRange = far - near;

    simd_float4x4 projectionMatrix = (simd_float4x4){{
        { xScale,  0.0f,   0.0f,               0.0f },
        { 0.0f,    yScale, 0.0f,               0.0f },
        { 0.0f,    0.0f,   -(far) / zRange,   -1.0f }, // Note: Right-handed projection
        { 0.0f,    0.0f,   -(far*near)/zRange, 0.0f }
    }};
    
    // Cube sits at the center of the world
    simd_float3 cubePos = { 0.0f, 0.0f, 0.0f };

    // Camera is at (0, 2, 5)
    // It's 5 units away from the screen and 2 units "above" the cube
    simd_float3 cameraPos = { 0.0f, 2.0f, 5.0f };

    // "Up" is toward the top of the sky
    simd_float3 upVector = { 0.0f, 1.0f, 0.0f };

    // --- Model-View (Look-At) Matrix ---
    // Parameters: cameraPos, cubePos (target), upVector
    simd_float3 zAxis = simd_normalize(cameraPos - cubePos); // Forward
    simd_float3 xAxis = simd_normalize(simd_cross(upVector, zAxis)); // Right
    simd_float3 yAxis = simd_cross(zAxis, xAxis); // Up

    simd_float4x4 modelViewMatrix = (simd_float4x4){{
        { xAxis.x, yAxis.x, zAxis.x, 0.0f },
        { xAxis.y, yAxis.y, zAxis.y, 0.0f },
        { xAxis.z, yAxis.z, zAxis.z, 0.0f },
        { -simd_dot(xAxis, cameraPos), -simd_dot(yAxis, cameraPos), -simd_dot(zAxis, cameraPos), 1.0f }
    }};
    
    uniforms.projectionMatrix = projectionMatrix;
    uniforms.modelViewMatrix = modelViewMatrix;
    
    void* pBufferContents = _pUniformsBuffer->contents();
    std::memcpy(pBufferContents, &uniforms, sizeof(uniforms));
}

Renderer::~Renderer()
{
    _pDefaultLibrary->release();
    _pCommandBuffer->release();
    _pCommandQueue->release();
    _pDevice->release();
}

std::vector<MTL::Buffer*> Renderer::makeTriangleDataBuffers(uint count) {
    std::vector<MTL::Buffer*> bufferArray;
    bufferArray.reserve(count);
    
    for (uint32_t bufferNumber = 0; bufferNumber < count; ++bufferNumber) {
        MTL::Buffer* pBuffer = _pDevice->newBuffer(sizeof(CubeData), MTL::ResourceStorageModeShared);
        if (!pBuffer)
            continue;
        bufferArray.push_back(pBuffer);
    }
    
    return bufferArray;
}

MTL4::ArgumentTable* Renderer::makeArgumentTable() {
    NS::Error* pError = nullptr;
    MTL4::ArgumentTableDescriptor* pDesc = MTL4::ArgumentTableDescriptor::alloc()->init();
    pDesc->setMaxBufferBindCount(3);
    MTL4::ArgumentTable* pArgumentTable = _pDevice->newArgumentTable(pDesc, &pError);
    pDesc->release();
    return pArgumentTable;
}

MTL::ResidencySet* Renderer::makeResidencySet() {
    NS::Error* pError = nullptr;
    MTL::ResidencySetDescriptor* pDesc = MTL::ResidencySetDescriptor::alloc()->init();
    MTL::ResidencySet* pResidencySet = _pDevice->newResidencySet(pDesc, &pError);
    pDesc->release();
    return pResidencySet;
}

std::vector<MTL4::CommandAllocator*> Renderer::makeCommandAllocators(uint count) {
    std::vector<MTL4::CommandAllocator*> allocatorArray;
    allocatorArray.reserve(count);
    for (uint32_t allocatorNumber = 0; allocatorNumber < count; ++allocatorNumber) {
        MTL4::CommandAllocator* pAllocator = _pDevice->newCommandAllocator();
        if (pAllocator) {
            allocatorArray.push_back(pAllocator);
        }
    }
    return allocatorArray;
}

void Renderer::updateViewportSize(const simd::float2& size) {
    _viewportSize.x = size.x;
    _viewportSize.y = size.y;
    void* pBufferContents = _pViewportSizeBuffer->contents();
    std::memcpy(pBufferContents, &_viewportSize, sizeof(_viewportSize));
}

MTL4::Compiler* Renderer::createDefaultMetalCompiler() {
    NS::Error* pError = nullptr;
    MTL4::CompilerDescriptor* pDesc = MTL4::CompilerDescriptor::alloc()->init();
    MTL4::Compiler* pCompiler = _pDevice->newCompiler(pDesc, &pError);
    pDesc->release();
    return pCompiler;
}

MTL::RenderPipelineState* Renderer::compileRenderPipeline(MTL::PixelFormat colorPixelFormat) {
    NS::Error* pError = nullptr;

    MTL::RenderPipelineDescriptor* pDesc = MTL::RenderPipelineDescriptor::alloc()->init();
    pDesc->setLabel(NS::String::string("Basic Metal render pipeline", NS::UTF8StringEncoding));
    pDesc->colorAttachments()->object(0)->setPixelFormat(colorPixelFormat);
    pDesc->setDepthAttachmentPixelFormat(MTL::PixelFormatDepth32Float);
    
    MTL::Function* pVertexFunc = _pDefaultLibrary->newFunction(NS::String::string("vertexShader", NS::UTF8StringEncoding));
    MTL::Function* pFragmentFunc = _pDefaultLibrary->newFunction(NS::String::string("fragmentShader", NS::UTF8StringEncoding));

    // 3. Configure the descriptor
    pDesc->setVertexFunction(pVertexFunc);
    pDesc->setFragmentFunction(pFragmentFunc);

    // 4. Compile the state
    MTL::RenderPipelineState* pPipelineState = _pDevice->newRenderPipelineState(pDesc, &pError);

    // 5. Validation
    if (!pPipelineState) {
        // Log the error using the localized description
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

void Renderer::waitOnSharedEvent(MTL::SharedEvent* pSharedEvent, uint64_t earlierFrameNumber) {
    // 1. Define the timeout in milliseconds
    const uint64_t tenMilliseconds = 10;
    
    // 2. Perform the blocking wait on the CPU thread
    // Returns true if the signal was received before the timeout
    bool beforeTimeout = pSharedEvent->waitUntilSignaledValue(earlierFrameNumber, tenMilliseconds);
    
    // 3. Handle timeout or error logging
    if (!beforeTimeout) {
        std::cerr << "No signal from frame " << earlierFrameNumber << " to shared event after " << tenMilliseconds << "ms" << std::endl;
    }
}


void Renderer::draw(MTK::View* pView)
{
    _frameNumber += 1;

    const uint32_t frameIndex = _frameNumber % kMaxFramesInFlight;
    if (_frameNumber > kMaxFramesInFlight) {
        waitOnSharedEvent(_pSharedEvent, _frameNumber - kMaxFramesInFlight);
    }

    MTL4::CommandAllocator* pFrameAllocator = _commandAllocators[frameIndex];
    pFrameAllocator->reset();
    
    // 6. Begin Command Buffer recording
    // Assuming _pCommandBuffer is a member of type MTL::CommandBuffer*
    _pCommandBuffer->beginCommandBuffer(pFrameAllocator);
//    _pCommandBuffer->setLabel(pLabel); // TODO: Add here setup for frame numbder
    
    // 7. Initialize Render Pass
    // Metal 4 specific call: currentMTL4RenderPassDescriptor
    MTL4::RenderPassDescriptor* pPassDesc = pView->currentMTL4RenderPassDescriptor();
    pPassDesc->depthAttachment()->setLoadAction(MTL::LoadActionClear);
    pPassDesc->depthAttachment()->setStoreAction(MTL::StoreActionDontCare);
    pPassDesc->depthAttachment()->setClearDepth(1.0);
    MTL4::RenderCommandEncoder* pRenderEncoder = _pCommandBuffer->renderCommandEncoder(pPassDesc);
    
//    pRenderEncoder->setLabel(pLabel);
    
    // 8. Configure Pipeline and Arguments
    pRenderEncoder->setRenderPipelineState(_pRenderPipelineState);
    
    setViewportSize(_viewportSize, pRenderEncoder);
    this->setRenderPassArguments(pRenderEncoder,
                                 _frameNumber,
                                 _pArgumentTable,
                                 _triangleVertexBuffers[frameIndex],
                                 _pViewportSizeBuffer);
    
    // 9. Draw the triangle
    pRenderEncoder->drawPrimitives(MTL::PrimitiveTypeTriangle, (NS::UInteger)0, (NS::UInteger)36);
    
    // 10. Finalize Encoding and Submit
    pRenderEncoder->endEncoding();
    _pCommandBuffer->endCommandBuffer();
    
    this->submitCommandBuffer(_pCommandBuffer, _pCommandQueue, pView);
    
    // 11. Signal completion to the Shared Event
    _pCommandQueue->signalEvent(_pSharedEvent, _frameNumber);
}

void Renderer::setRenderPassArguments(MTL4::RenderCommandEncoder* pRenderEncoder,
                            NS::UInteger frameNumber,
                            MTL4::ArgumentTable* pArgumentTable,
                            MTL::Buffer* pVertexBuffer,
                            MTL::Buffer* pViewportSizeBuffer)
{
    float angle = frameNumber * 0.0002f; // Rotation speed
    simd_float4x4 rotationY = (simd_float4x4){{
        { cosf(angle),  0.0f, -sinf(angle), 0.0f },
        { 0.0f,         1.0f,  0.0f,        0.0f },
        { sinf(angle),  0.0f,  cosf(angle), 0.0f },
        { 0.0f,         0.0f,  0.0f,        1.0f }
    }};

    // Update uniforms to include rotation
    uniforms.modelViewMatrix = simd_mul(uniforms.modelViewMatrix, rotationY);
    void* pBufferContents = _pUniformsBuffer->contents();
    std::memcpy(pBufferContents, &uniforms, sizeof(uniforms));
    
    configureVertexDataForBufferCube(frameNumber, pVertexBuffer->contents());
    pArgumentTable->setAddress(pVertexBuffer->gpuAddress(), 0);
    pArgumentTable->setAddress(pViewportSizeBuffer->gpuAddress(), 1);
    pArgumentTable->setAddress(_pUniformsBuffer->gpuAddress(), 2);
    pRenderEncoder->setArgumentTable(pArgumentTable, MTL::RenderStageVertex);
    pRenderEncoder->setDepthStencilState(pDepthState);
    pRenderEncoder->setCullMode(MTL::CullModeFront);
}

void Renderer::setViewportSize(simd_uint2 size, MTL4::RenderCommandEncoder* pRenderEncoder) {
    // 1. Initialize the viewport struct
    // MTLViewport is a C-struct, so initialization is straightforward
    MTL::Viewport viewport;
    viewport.originX = 0.0;
    viewport.originY = 0.0;
    viewport.znear   = 0.0;
    viewport.zfar    = 1.0;
    viewport.width   = static_cast<double>(size.x);
    viewport.height  = static_cast<double>(size.y);

    // 2. Set the viewport on the encoder
    pRenderEncoder->setViewport(viewport);
}

void Renderer::submitCommandBuffer(MTL4::CommandBuffer* pCommandBuffer,
                         MTL4::CommandQueue* pCommandQueue,
                         MTK::View* pView)
{
    // 1. Retrieve the current drawable from the view
    // Note: MTK::View returns a CA::MetalDrawable*
    CA::MetalDrawable* pCurrentDrawable = pView->currentDrawable();

    if (!pCurrentDrawable) {
        return;
    }

    // 2. Instruct the queue to wait until the drawable is ready
    pCommandQueue->wait(pCurrentDrawable);

    // 3. Submit the command buffer(s) to the queue
    // Metal 4 uses an array-based commit. We pass the address of our single buffer.
    MTL4::CommandBuffer* commandBuffers[] = { pCommandBuffer };
    pCommandQueue->commit(commandBuffers, 1);

    // 4. Signal that the GPU is finished with the render pass for this drawable
    pCommandQueue->signalDrawable(pCurrentDrawable);

    // 5. Present the drawable on the display
    pCurrentDrawable->present();
}


//MTL::RenderPipelineState* Renderer::compileRenderPipeline(MTL::PixelFormat colorPixelFormat) {
//    NS::Error* pError = nullptr;
//    
//    MTL4::Compiler* pCompiler = createDefaultMetalCompiler();
//    
//    // 2. Configure the Render Pipeline Descriptor
//    MTL4::RenderPipelineDescriptor* pPipelineDesc = MTL4::RenderPipelineDescriptor::alloc()->init();
//    pPipelineDesc->setLabel(NS::String::string("Basic Metal 4 render pipeline", NS::UTF8StringEncoding));
//    pPipelineDesc->colorAttachments()->object(0)->setPixelFormat(colorPixelFormat);
//    
//    // 3. Configure Vertex Shader Function Descriptor
//    MTL4::LibraryFunctionDescriptor* pVertexFuncDesc = MTL4::LibraryFunctionDescriptor::alloc()->init();
//    pVertexFuncDesc->setLibrary(_pDefaultLibrary);
//    pVertexFuncDesc->setName(NS::String::string("vertexShader", NS::UTF8StringEncoding));
//    pPipelineDesc->setVertexFunctionDescriptor(pVertexFuncDesc);
//    
//    // 4. Configure Fragment Shader Function Descriptor
//    MTL4::LibraryFunctionDescriptor* pFragmentFuncDesc = MTL4::LibraryFunctionDescriptor::alloc()->init();
//    pFragmentFuncDesc->setLibrary(_pDefaultLibrary);
//    pFragmentFuncDesc->setName(NS::String::string("fragmentShader", NS::UTF8StringEncoding));
//    pPipelineDesc->setFragmentFunctionDescriptor(pFragmentFuncDesc);
//    
//    // 5. Set Compiler Task Options (Assuming default options)
//    MTL4::CompilerTaskOptions* pCompilerTaskOptions = MTL4::CompilerTaskOptions::alloc()->init();
//    
//    auto pMainBundle = NS::Bundle::mainBundle();
//    auto pResourceName = NS::String::string("archive.metallib", NS::UTF8StringEncoding);
//    auto pExtension = NS::String::string("metallib", NS::UTF8StringEncoding);
//    
//    NS::URL* pArchiveURL = pMainBundle->URLForAuxiliaryExecutable(pResourceName);
//    
//    if (!pArchiveURL) {
//        return nullptr;
//    }
//    
//    MTL4::Archive* pDefaultArchive = _pDevice->newArchive(pArchiveURL, &pError);
//    
//    NS::Object* pArchiveObj = (NS::Object*)pDefaultArchive;
//    NS::Array* pArchiveArray = NS::Array::array(pArchiveObj);
//    
//    pCompilerTaskOptions->setLookupArchives(pArchiveArray);
//    pDefaultArchive->release();
//    
//    // 6. Synchronously compile the pipeline state
//    MTL::RenderPipelineState* pPipelineState = pCompiler->newRenderPipelineState(
//                                                                                 pPipelineDesc,
//                                                                                 pCompilerTaskOptions,
//                                                                                 &pError
//                                                                                 );
//        if (!pPipelineState) {
//        printf("Compiler Error: %s\n", pError->localizedDescription()->utf8String());
//        assert(pPipelineState != nullptr);
//    }
//    
//    pCompilerTaskOptions->release();
//    pFragmentFuncDesc->release();
//    pVertexFuncDesc->release();
//    pPipelineDesc->release();
//    pCompiler->release();
//    
//    return pPipelineState;
//}


/// A four-component red color.
const simd_float4 red = { 1.0, 0.0, 0.0, 1.0 };

/// A four-component green color.
const simd_float4 green = { 0.0, 1.0, 0.0, 1.0 };

/// A four-component blue color.
const simd_float4 blue = { 0.0, 0.0, 1.0, 1.0 };


/// Configures an equilateral triangle's vertex data.
///
/// The function assigns:
/// - Red, green, and blue, to the first, second, and third vertices, respectively
/// - The positions along on a circle that circumscribes the triangle,
/// which are 120° apart from each other
///
/// - Parameters:
///   - radius: The radius of the circle that circumscribes the equilateral triangle.
///   - rotationInDegrees: An angle of rotation for the triangle, in degrees.
///   - triangleData: A pointer to a triangle data instance.
void triangleRedGreenBlue(float radius,
                          float rotationInDegrees,
                          TriangleData *triangleData)
{
    /// An angle, in radians, that's equal to the rotation.
    const float angle0 = (float)rotationInDegrees * M_PI / 180.0f;

    /// An angle, in radians, one-third of a circle more than the previous angle.
    ///
    /// This is the equivalent of adding 120° to the first angle.
    const float angle1 = angle0 + (2.0f * M_PI  / 3.0f);

    /// An angle, in radians, one-third of a circle more than the previous angle.
    ///
    /// This is the equivalent of adding 240° to the first angle.
    const float angle2 = angle0 + (4.0f * M_PI  / 3.0f);

    /// The position of the triangle's first vertex.
    simd_float2 position0 = {
        radius * cosf(angle0),
        radius * sinf(angle0)
    };

    /// The position of the triangle's second vertex.
    simd_float2 position1 = {
        radius * cosf(angle1),
        radius * sinf(angle1)
    };

    /// The position of the triangle's third vertex.
    simd_float2 position2 = {
        radius * cosf(angle2),
        radius * sinf(angle2)
    };

//    // The triangle's red, bottom-right vertex.
//    triangleData->vertex0.color = red;
//    triangleData->vertex0.position = position0;
//
//    // The triangle's green, bottom-left vertex.
//    triangleData->vertex1.color = green;
//    triangleData->vertex1.position = position1;
//
//    // The triangle's blue, top-center vertex.
//    triangleData->vertex2.color = blue;
//    triangleData->vertex2.position = position2;
}

/// Configures an equilateral triangle's vertex data
/// and copies the bytes into a Metal buffer's pointer.
///
/// - Parameters:
///   - rotationInDegrees: An angle of rotation for the triangle, in degrees.
///   - bufferContents: A pointer from an `MTLBuffer` instance's `contents` property.
void configureVertexDataForBuffer(long rotationInDegrees,
                                  void *bufferContents)
{
    const short radius = 350;
    const short angle = rotationInDegrees % 360;

    TriangleData triangleData;
    triangleRedGreenBlue(radius, (float)angle, &triangleData);

    // Update the buffer that stores the triangle data.
    memcpy(bufferContents, &triangleData, sizeof(TriangleData));
}

void configureCubeData(float size, CubeData *cubeData) {
    float s = size / 2.0f;

    // Define the 8 unique corners of a cube (x, y, z)
    simd_float3 v[] = {
        {-s, -s,  s}, { s, -s,  s}, { s,  s,  s}, {-s,  s,  s}, // Front corners (0-3)
        {-s, -s, -s}, { s, -s, -s}, { s,  s, -s}, {-s,  s, -s}  // Back corners (4-7)
    };

    // Define 6 distinct colors for the faces
    simd_float4 colors[] = {
        {1, 0, 0, 1}, {0, 1, 0, 1}, {0, 0, 1, 1}, // Red, Green, Blue
        {1, 1, 0, 1}, {1, 0, 1, 1}, {0, 1, 1, 1}  // Yellow, Magenta, Cyan
    };

    int i = 0;
    // Helper lambda to add two triangles (one square face)
    auto addFace = [&](int a, int b, int c, int d, simd_float4 color) {
        // Triangle 1
        cubeData->vertices[i++] = { v[a], color };
        cubeData->vertices[i++] = { v[b], color };
        cubeData->vertices[i++] = { v[c], color };
        // Triangle 2
        cubeData->vertices[i++] = { v[a], color };
        cubeData->vertices[i++] = { v[c], color };
        cubeData->vertices[i++] = { v[d], color };
    };

    // Define faces by corner indices
    addFace(0, 1, 2, 3, colors[0]); // Front  (+Z)
    addFace(5, 4, 7, 6, colors[1]); // Back   (-Z)
    addFace(4, 0, 3, 7, colors[2]); // Left   (-X)
    addFace(1, 5, 6, 2, colors[3]); // Right  (+X)
    addFace(3, 2, 6, 7, colors[4]); // Top    (+Y)
    addFace(4, 5, 1, 0, colors[5]); // Bottom (-Y)
}

void configureVertexDataForBufferCube(long frameNumber, void *bufferContents) {
    const float cubeSize = 1.0f;
    
    CubeData data;
    configureCubeData(cubeSize, &data);
    
    // Copy to the Metal Buffer
    std::memcpy(bufferContents, &data, sizeof(CubeData));
}
