#include "rhi-replay.h"
#include "replay-utils.h"

#include <slang-replay-api.h>
#include <slang-rhi.h>

#include <cstring>
#include <unordered_map>
#include <vector>
#include <string>

namespace rhi::replay {

// =============================================================================
// Handler dispatch table
// =============================================================================

using Handler = bool (*)();

static std::unordered_map<std::string, Handler>& getHandlerMap()
{
    static std::unordered_map<std::string, Handler> map;
    return map;
}

static void reg(const char* sig, Handler handler)
{
    getHandlerMap()[sig] = handler;
}

// =============================================================================
// Static/free function handlers
// =============================================================================

static bool handleCreateDevice()
{
    auto desc = readDesc<DeviceDesc>();

    // Null out pointer fields that can't survive POD serialization.
    // SlangContext::initialize will create fresh objects for null pointers.
    desc.adapter = nullptr;
    desc.adapterLUID = nullptr;
    desc.requiredFeatures = nullptr;
    desc.requiredFeatureCount = 0;
    desc.slang.slangGlobalSession = nullptr;
    desc.slang.searchPaths = nullptr;
    desc.slang.searchPathCount = 0;
    desc.slang.preprocessorMacros = nullptr;
    desc.slang.preprocessorMacroCount = 0;
    desc.slang.compilerOptionEntries = nullptr;
    desc.slang.compilerOptionEntryCount = 0;
    desc.slang.targetProfile = nullptr;
    desc.persistentShaderCache = nullptr;
    desc.persistentPipelineCache = nullptr;

    ComPtr<IDevice> device;
    rhi::getRHI()->createDevice(desc, device.writeRef());
    mapOutputObject(device);
    readReturnValue();
    return true;
}

// =============================================================================
// IDevice handlers
// =============================================================================

static bool handleDeviceCreateBuffer()
{
    auto* device = getObj<IDevice>(slangReplay_getCurrentThisHandle());
    auto desc = readDesc<BufferDesc>();
    ISlangBlob* initBlob = slangReplay_readBlob();
    const void* initData = initBlob ? initBlob->getBufferPointer() : nullptr;

    IBuffer* outBuffer = nullptr;
    device->createBuffer(desc, initData, &outBuffer);
    mapOutputObject(outBuffer);
    readReturnValue();
    if (initBlob)
        initBlob->release();
    return true;
}

static bool handleDeviceCreateBufferFromNativeHandle()
{
    auto* device = getObj<IDevice>(slangReplay_getCurrentThisHandle());
    auto handle = readPOD<NativeHandle>();
    auto desc = readDesc<BufferDesc>();

    IBuffer* outBuffer = nullptr;
    device->createBufferFromNativeHandle(handle, desc, &outBuffer);
    mapOutputObject(outBuffer);
    readReturnValue();
    return true;
}

static bool handleDeviceCreateBufferFromSharedHandle()
{
    auto* device = getObj<IDevice>(slangReplay_getCurrentThisHandle());
    auto handle = readPOD<NativeHandle>();
    auto desc = readDesc<BufferDesc>();

    IBuffer* outBuffer = nullptr;
    device->createBufferFromSharedHandle(handle, desc, &outBuffer);
    mapOutputObject(outBuffer);
    readReturnValue();
    return true;
}

static bool handleDeviceCreateTexture()
{
    auto* device = getObj<IDevice>(slangReplay_getCurrentThisHandle());
    auto desc = readDesc<TextureDesc>();

    uint32_t subresourceCount = slangReplay_readUInt32();

    std::vector<SubresourceData> subresources(subresourceCount);
    std::vector<Slang::ComPtr<ISlangBlob>> blobStorage(subresourceCount);

    for (uint32_t i = 0; i < subresourceCount; i++)
    {
        subresources[i].rowPitch = readPOD<Size>();
        subresources[i].slicePitch = readPOD<Size>();
        ISlangBlob* blob = slangReplay_readBlob();
        blobStorage[i] = blob;
        subresources[i].data = blob ? blob->getBufferPointer() : nullptr;
    }

    const SubresourceData* initData = subresourceCount > 0 ? subresources.data() : nullptr;

    ITexture* outTexture = nullptr;
    device->createTexture(desc, initData, &outTexture);
    mapOutputObject(outTexture);
    readReturnValue();
    return true;
}

static bool handleDeviceCreateTextureFromNativeHandle()
{
    auto* device = getObj<IDevice>(slangReplay_getCurrentThisHandle());
    auto handle = readPOD<NativeHandle>();
    auto desc = readDesc<TextureDesc>();

    ITexture* outTexture = nullptr;
    device->createTextureFromNativeHandle(handle, desc, &outTexture);
    mapOutputObject(outTexture);
    readReturnValue();
    return true;
}

static bool handleDeviceCreateTextureFromSharedHandle()
{
    auto* device = getObj<IDevice>(slangReplay_getCurrentThisHandle());
    auto handle = readPOD<NativeHandle>();
    auto desc = readDesc<TextureDesc>();
    auto size = readPOD<Size>();

    ITexture* outTexture = nullptr;
    device->createTextureFromSharedHandle(handle, desc, size, &outTexture);
    mapOutputObject(outTexture);
    readReturnValue();
    return true;
}

static bool handleDeviceMapBuffer()
{
    auto* device = getObj<IDevice>(slangReplay_getCurrentThisHandle());
    auto* buffer = readObjFromStream<IBuffer>();
    auto mode = readPOD<CpuAccessMode>();

    void* outData = nullptr;
    device->mapBuffer(buffer, mode, &outData);
    readReturnValue();
    return true;
}

static bool handleDeviceUnmapBuffer()
{
    auto* device = getObj<IDevice>(slangReplay_getCurrentThisHandle());
    auto* buffer = readObjFromStream<IBuffer>();

    // If buffer was mapped for write, the recorded data blob follows.
    // We need to write this data into the mapped buffer before unmapping.
    // However, we don't know at this point whether a blob was recorded —
    // that depends on CpuAccessMode::Write having been used during mapBuffer.
    // For now, we just unmmap. TODO: handle write-mapped data replay.
    device->unmapBuffer(buffer);
    readReturnValue();
    return true;
}

static bool handleDeviceCreateSampler()
{
    auto* device = getObj<IDevice>(slangReplay_getCurrentThisHandle());
    auto desc = readDesc<SamplerDesc>();

    ISampler* outSampler = nullptr;
    device->createSampler(desc, &outSampler);
    mapOutputObject(outSampler);
    readReturnValue();
    return true;
}

static bool handleDeviceCreateTextureView()
{
    auto* device = getObj<IDevice>(slangReplay_getCurrentThisHandle());
    auto* texture = readObjFromStream<ITexture>();
    auto desc = readDesc<TextureViewDesc>();

    ITextureView* outView = nullptr;
    device->createTextureView(texture, desc, &outView);
    mapOutputObject(outView);
    readReturnValue();
    return true;
}

static bool handleDeviceCreateAccelerationStructure()
{
    auto* device = getObj<IDevice>(slangReplay_getCurrentThisHandle());
    auto desc = readDesc<AccelerationStructureDesc>();

    IAccelerationStructure* outAS = nullptr;
    device->createAccelerationStructure(desc, &outAS);
    mapOutputObject(outAS);
    readReturnValue();
    return true;
}

static bool handleDeviceCreateSurface()
{
    auto* device = getObj<IDevice>(slangReplay_getCurrentThisHandle());

    ISurface* outSurface = nullptr;
    device->createSurface({}, &outSurface);
    mapOutputObject(outSurface);
    readReturnValue();
    return true;
}

static bool handleDeviceCreateInputLayout()
{
    auto* device = getObj<IDevice>(slangReplay_getCurrentThisHandle());
    auto desc = readDesc<InputLayoutDesc>();

    std::vector<InputElementDesc> elements(desc.inputElementCount);
    for (uint32_t i = 0; i < desc.inputElementCount; i++)
    {
        elements[i].semanticName = slangReplay_readString();
        elements[i].semanticIndex = slangReplay_readUInt32();
        elements[i].format = readPOD<Format>();
        elements[i].offset = slangReplay_readUInt32();
        elements[i].bufferSlotIndex = slangReplay_readUInt32();
    }
    desc.inputElements = elements.data();

    auto vertexStreams = readPODArray<VertexStreamDesc>();
    desc.vertexStreams = vertexStreams.data();
    desc.vertexStreamCount = static_cast<uint32_t>(vertexStreams.size());

    IInputLayout* outLayout = nullptr;
    device->createInputLayout(desc, &outLayout);
    mapOutputObject(outLayout);
    readReturnValue();
    return true;
}

static bool handleDeviceGetQueue()
{
    auto* device = getObj<IDevice>(slangReplay_getCurrentThisHandle());
    auto type = readPOD<QueueType>();

    ICommandQueue* outQueue = nullptr;
    device->getQueue(type, &outQueue);
    mapOutputObject(outQueue);
    readReturnValue();
    return true;
}

static bool handleDeviceCreateShaderObject()
{
    auto* device = getObj<IDevice>(slangReplay_getCurrentThisHandle());

    // createShaderObject takes slang::ISession* and TypeReflection*, which
    // aren't serialized individually. Skip and create via alternate path.
    IShaderObject* outObject = nullptr;
    device->createShaderObject(nullptr, nullptr, ShaderObjectContainerType::None, &outObject);
    mapOutputObject(outObject);
    readReturnValue();
    return true;
}

static bool handleDeviceCreateShaderObjectFromTypeLayout()
{
    auto* device = getObj<IDevice>(slangReplay_getCurrentThisHandle());

    IShaderObject* outObject = nullptr;
    device->createShaderObjectFromTypeLayout(nullptr, &outObject);
    mapOutputObject(outObject);
    readReturnValue();
    return true;
}

static bool handleDeviceCreateRootShaderObject()
{
    auto* device = getObj<IDevice>(slangReplay_getCurrentThisHandle());
    auto* program = readObjFromStream<IShaderProgram>();

    IShaderObject* outObject = nullptr;
    device->createRootShaderObject(program, &outObject);
    mapOutputObject(outObject);
    readReturnValue();
    return true;
}

static bool handleDeviceCreateShaderProgram()
{
    auto* device = getObj<IDevice>(slangReplay_getCurrentThisHandle());
    auto desc = readDesc<ShaderProgramDesc>();
    auto* slangGlobalScope = readObjFromStream<slang::IComponentType>();
    desc.slangGlobalScope = slangGlobalScope;

    uint32_t entryPointCount = slangReplay_readUInt32();
    std::vector<slang::IComponentType*> entryPoints(entryPointCount);
    for (uint32_t i = 0; i < entryPointCount; i++)
        entryPoints[i] = readObjFromStream<slang::IComponentType>();
    desc.slangEntryPoints = entryPoints.data();
    desc.slangEntryPointCount = entryPointCount;

    IShaderProgram* outProgram = nullptr;
    device->createShaderProgram(desc, &outProgram, nullptr);
    mapOutputObject(outProgram);
    readReturnValue();
    return true;
}

static bool handleDeviceCreateRenderPipeline()
{
    auto* device = getObj<IDevice>(slangReplay_getCurrentThisHandle());
    auto desc = readDesc<RenderPipelineDesc>();
    desc.program = readObjFromStream<IShaderProgram>();
    desc.inputLayout = readObjFromStream<IInputLayout>();

    auto targets = readPODArray<ColorTargetDesc>();
    desc.targets = targets.data();
    desc.targetCount = static_cast<uint32_t>(targets.size());

    desc.label = slangReplay_readString();

    IRenderPipeline* outPipeline = nullptr;
    device->createRenderPipeline(desc, &outPipeline);
    mapOutputObject(outPipeline);
    readReturnValue();
    return true;
}

static bool handleDeviceCreateComputePipeline()
{
    auto* device = getObj<IDevice>(slangReplay_getCurrentThisHandle());
    auto desc = readDesc<ComputePipelineDesc>();

    IComputePipeline* outPipeline = nullptr;
    device->createComputePipeline(desc, &outPipeline);
    mapOutputObject(outPipeline);
    readReturnValue();
    return true;
}

static bool handleDeviceCreateRayTracingPipeline()
{
    auto* device = getObj<IDevice>(slangReplay_getCurrentThisHandle());
    auto desc = readDesc<RayTracingPipelineDesc>();
    desc.program = readObjFromStream<IShaderProgram>();

    uint32_t hitGroupCount = slangReplay_readUInt32();
    std::vector<HitGroupDesc> hitGroups(hitGroupCount);
    for (uint32_t i = 0; i < hitGroupCount; i++)
    {
        hitGroups[i].hitGroupName = slangReplay_readString();
        hitGroups[i].closestHitEntryPoint = slangReplay_readString();
        hitGroups[i].anyHitEntryPoint = slangReplay_readString();
        hitGroups[i].intersectionEntryPoint = slangReplay_readString();
    }
    desc.hitGroups = hitGroups.data();
    desc.hitGroupCount = hitGroupCount;

    desc.label = slangReplay_readString();

    IRayTracingPipeline* outPipeline = nullptr;
    device->createRayTracingPipeline(desc, &outPipeline);
    mapOutputObject(outPipeline);
    readReturnValue();
    return true;
}

static bool handleDeviceReadTexture()
{
    auto* device = getObj<IDevice>(slangReplay_getCurrentThisHandle());
    auto* texture = readObjFromStream<ITexture>();
    uint32_t layer = slangReplay_readUInt32();
    uint32_t mip = slangReplay_readUInt32();

    ISlangBlob* outBlob = nullptr;
    SubresourceLayout outLayout{};
    device->readTexture(texture, layer, mip, &outBlob, &outLayout);
    if (outBlob)
        outBlob->release();
    return true;
}

static bool handleDeviceReadTextureBlob()
{
    auto* device = getObj<IDevice>(slangReplay_getCurrentThisHandle());
    auto* texture = readObjFromStream<ITexture>();
    uint32_t layer = slangReplay_readUInt32();
    uint32_t mip = slangReplay_readUInt32();

    ISlangBlob* outBlob = nullptr;
    SubresourceLayout outLayout{};
    device->readTexture(texture, layer, mip, &outBlob, &outLayout);
    if (outBlob)
        outBlob->release();
    return true;
}

static bool handleDeviceReadBuffer()
{
    auto* device = getObj<IDevice>(slangReplay_getCurrentThisHandle());
    auto* buffer = readObjFromStream<IBuffer>();
    auto offset = readPOD<Offset>();
    auto size = readPOD<Size>();

    ISlangBlob* outBlob = nullptr;
    device->readBuffer(buffer, offset, size, &outBlob);
    if (outBlob)
        outBlob->release();
    return true;
}

static bool handleDeviceReadBufferBlob()
{
    auto* device = getObj<IDevice>(slangReplay_getCurrentThisHandle());
    auto* buffer = readObjFromStream<IBuffer>();
    auto offset = readPOD<Offset>();
    auto size = readPOD<Size>();

    ISlangBlob* outBlob = nullptr;
    device->readBuffer(buffer, offset, size, &outBlob);
    if (outBlob)
        outBlob->release();
    return true;
}

static bool handleDeviceCreateQueryPool()
{
    auto* device = getObj<IDevice>(slangReplay_getCurrentThisHandle());
    auto desc = readDesc<QueryPoolDesc>();

    IQueryPool* outPool = nullptr;
    device->createQueryPool(desc, &outPool);
    mapOutputObject(outPool);
    readReturnValue();
    return true;
}

static bool handleDeviceCreateFence()
{
    auto* device = getObj<IDevice>(slangReplay_getCurrentThisHandle());
    auto desc = readDesc<FenceDesc>();

    IFence* outFence = nullptr;
    device->createFence(desc, &outFence);
    mapOutputObject(outFence);
    readReturnValue();
    return true;
}

static bool handleDeviceWaitForFences()
{
    auto* device = getObj<IDevice>(slangReplay_getCurrentThisHandle());
    uint32_t fenceCount = slangReplay_readUInt32();
    bool waitForAll = slangReplay_readBool();
    uint64_t timeout = slangReplay_readUInt64();

    device->waitForFences(fenceCount, nullptr, nullptr, waitForAll, timeout);
    readReturnValue();
    return true;
}

static bool handleDeviceCreateHeap()
{
    auto* device = getObj<IDevice>(slangReplay_getCurrentThisHandle());
    auto desc = readPOD<HeapDesc>();

    IHeap* outHeap = nullptr;
    device->createHeap(desc, &outHeap);
    mapOutputObject(outHeap);
    readReturnValue();
    return true;
}

static bool handleDeviceConvertCooperativeVectorMatrix()
{
    auto* device = getObj<IDevice>(slangReplay_getCurrentThisHandle());
    auto dstBufferSize = readPOD<size_t>();
    auto srcBufferSize = readPOD<size_t>();
    uint32_t matrixCount = slangReplay_readUInt32();

    ISlangBlob* srcBlob = slangReplay_readBlob();
    const void* srcBuffer = srcBlob ? srcBlob->getBufferPointer() : nullptr;

    std::vector<CooperativeVectorMatrixDesc> dstDescs(matrixCount);
    std::vector<CooperativeVectorMatrixDesc> srcDescs(matrixCount);
    for (uint32_t i = 0; i < matrixCount; i++)
    {
        dstDescs[i] = readPOD<CooperativeVectorMatrixDesc>();
        srcDescs[i] = readPOD<CooperativeVectorMatrixDesc>();
    }

    std::vector<uint8_t> dstBuffer(dstBufferSize);
    device->convertCooperativeVectorMatrix(
        dstBuffer.data(),
        dstBufferSize,
        dstDescs.data(),
        srcBuffer,
        srcBufferSize,
        srcDescs.data(),
        matrixCount);

    readReturnValue();
    if (srcBlob)
        srcBlob->release();
    return true;
}

static bool handleDeviceCreateShaderTable()
{
    auto* device = getObj<IDevice>(slangReplay_getCurrentThisHandle());
    auto desc = readDesc<ShaderTableDesc>();
    desc.program = readObjFromStream<IShaderProgram>();

    auto rayGenNames = readStringArray();
    auto rayGenOverwrites = readPODArray<ShaderRecordOverwrite>();
    desc.rayGenShaderEntryPointNames = rayGenNames.data();
    desc.rayGenShaderRecordOverwrites = rayGenOverwrites.data();
    desc.rayGenShaderCount = static_cast<uint32_t>(rayGenNames.size());

    auto missNames = readStringArray();
    auto missOverwrites = readPODArray<ShaderRecordOverwrite>();
    desc.missShaderEntryPointNames = missNames.data();
    desc.missShaderRecordOverwrites = missOverwrites.data();
    desc.missShaderCount = static_cast<uint32_t>(missNames.size());

    auto hitGroupNames = readStringArray();
    auto hitGroupOverwrites = readPODArray<ShaderRecordOverwrite>();
    desc.hitGroupNames = hitGroupNames.data();
    desc.hitGroupRecordOverwrites = hitGroupOverwrites.data();
    desc.hitGroupCount = static_cast<uint32_t>(hitGroupNames.size());

    auto callableNames = readStringArray();
    auto callableOverwrites = readPODArray<ShaderRecordOverwrite>();
    desc.callableShaderEntryPointNames = callableNames.data();
    desc.callableShaderRecordOverwrites = callableOverwrites.data();
    desc.callableShaderCount = static_cast<uint32_t>(callableNames.size());

    IShaderTable* outTable = nullptr;
    device->createShaderTable(desc, &outTable);
    mapOutputObject(outTable);
    readReturnValue();
    return true;
}

// =============================================================================
// ICommandQueue handlers
// =============================================================================

static bool handleQueueCreateCommandEncoder()
{
    auto* queue = getObj<ICommandQueue>(slangReplay_getCurrentThisHandle());

    ICommandEncoder* outEncoder = nullptr;
    queue->createCommandEncoder(&outEncoder);
    mapOutputObject(outEncoder);
    readReturnValue();
    return true;
}

static bool handleQueueSubmit()
{
    auto* queue = getObj<ICommandQueue>(slangReplay_getCurrentThisHandle());

    uint32_t cmdBufCount = slangReplay_readUInt32();
    std::vector<ICommandBuffer*> cmdBuffers(cmdBufCount);
    for (uint32_t i = 0; i < cmdBufCount; i++)
        cmdBuffers[i] = readObjFromStream<ICommandBuffer>();

    uint32_t waitFenceCount = slangReplay_readUInt32();
    std::vector<IFence*> waitFences(waitFenceCount);
    std::vector<uint64_t> waitFenceValues(waitFenceCount);
    for (uint32_t i = 0; i < waitFenceCount; i++)
    {
        waitFences[i] = readObjFromStream<IFence>();
        waitFenceValues[i] = readPOD<uint64_t>();
    }

    uint32_t signalFenceCount = slangReplay_readUInt32();
    std::vector<IFence*> signalFences(signalFenceCount);
    std::vector<uint64_t> signalFenceValues(signalFenceCount);
    for (uint32_t i = 0; i < signalFenceCount; i++)
    {
        signalFences[i] = readObjFromStream<IFence>();
        signalFenceValues[i] = readPOD<uint64_t>();
    }

    SubmitDesc desc{};
    desc.commandBufferCount = cmdBufCount;
    desc.commandBuffers = cmdBuffers.data();
    desc.waitFenceCount = waitFenceCount;
    desc.waitFences = waitFences.data();
    desc.waitFenceValues = waitFenceValues.data();
    desc.signalFenceCount = signalFenceCount;
    desc.signalFences = signalFences.data();
    desc.signalFenceValues = signalFenceValues.data();

    queue->submit(desc);
    readReturnValue();
    return true;
}

static bool handleQueueWaitOnHost()
{
    auto* queue = getObj<ICommandQueue>(slangReplay_getCurrentThisHandle());
    queue->waitOnHost();
    readReturnValue();
    return true;
}

// =============================================================================
// ICommandEncoder handlers
// =============================================================================

static bool handleEncoderBeginRenderPass()
{
    auto* encoder = getObj<ICommandEncoder>(slangReplay_getCurrentThisHandle());

    uint32_t colorAttachmentCount = slangReplay_readUInt32();
    std::vector<RenderPassColorAttachment> colorAttachments(colorAttachmentCount);
    for (uint32_t i = 0; i < colorAttachmentCount; i++)
    {
        colorAttachments[i].view = readObjFromStream<ITextureView>();
        colorAttachments[i].resolveTarget = readObjFromStream<ITextureView>();
        colorAttachments[i].loadOp = readPOD<LoadOp>();
        colorAttachments[i].storeOp = readPOD<StoreOp>();
        slangReplay_readPOD(colorAttachments[i].clearValue, sizeof(colorAttachments[i].clearValue));
    }

    bool hasDepth = slangReplay_readBool();
    RenderPassDepthStencilAttachment depthStencil{};
    if (hasDepth)
    {
        depthStencil.view = readObjFromStream<ITextureView>();
        depthStencil.depthLoadOp = readPOD<LoadOp>();
        depthStencil.depthStoreOp = readPOD<StoreOp>();
        depthStencil.depthClearValue = readPOD<float>();
        depthStencil.depthReadOnly = slangReplay_readBool();
        depthStencil.stencilLoadOp = readPOD<LoadOp>();
        depthStencil.stencilStoreOp = readPOD<StoreOp>();
        depthStencil.stencilClearValue = readPOD<uint8_t>();
        depthStencil.stencilReadOnly = slangReplay_readBool();
    }

    RenderPassDesc desc{};
    desc.colorAttachments = colorAttachments.data();
    desc.colorAttachmentCount = colorAttachmentCount;
    desc.depthStencilAttachment = hasDepth ? &depthStencil : nullptr;

    auto* passEncoder = encoder->beginRenderPass(desc);
    (void)passEncoder;
    return true;
}

static bool handleEncoderBeginComputePass()
{
    auto* encoder = getObj<ICommandEncoder>(slangReplay_getCurrentThisHandle());
    auto* passEncoder = encoder->beginComputePass();
    (void)passEncoder;
    return true;
}

static bool handleEncoderBeginRayTracingPass()
{
    auto* encoder = getObj<ICommandEncoder>(slangReplay_getCurrentThisHandle());
    auto* passEncoder = encoder->beginRayTracingPass();
    (void)passEncoder;
    return true;
}

static bool handleEncoderCopyBuffer()
{
    auto* encoder = getObj<ICommandEncoder>(slangReplay_getCurrentThisHandle());
    auto* dst = readObjFromStream<IBuffer>();
    auto* src = readObjFromStream<IBuffer>();
    auto dstOffset = readPOD<Offset>();
    auto srcOffset = readPOD<Offset>();
    auto size = readPOD<Size>();
    encoder->copyBuffer(dst, dstOffset, src, srcOffset, size);
    return true;
}

static bool handleEncoderCopyTexture()
{
    auto* encoder = getObj<ICommandEncoder>(slangReplay_getCurrentThisHandle());
    auto* dst = readObjFromStream<ITexture>();
    auto* src = readObjFromStream<ITexture>();
    auto dstSubresource = readPOD<SubresourceRange>();
    auto dstOffset = readPOD<Offset3D>();
    auto srcSubresource = readPOD<SubresourceRange>();
    auto srcOffset = readPOD<Offset3D>();
    auto extent = readPOD<Extent3D>();
    encoder->copyTexture(dst, dstSubresource, dstOffset, src, srcSubresource, srcOffset, extent);
    return true;
}

static bool handleEncoderCopyTextureToBuffer()
{
    auto* encoder = getObj<ICommandEncoder>(slangReplay_getCurrentThisHandle());
    auto* dst = readObjFromStream<IBuffer>();
    auto* src = readObjFromStream<ITexture>();
    encoder->copyTextureToBuffer(dst, 0, 0, 0, src, 0, 0, {}, {});
    return true;
}

static bool handleEncoderCopyBufferToTexture()
{
    auto* encoder = getObj<ICommandEncoder>(slangReplay_getCurrentThisHandle());
    auto* dst = readObjFromStream<ITexture>();
    auto* src = readObjFromStream<IBuffer>();
    encoder->copyBufferToTexture(dst, 0, 0, {}, src, 0, 0, 0, {});
    return true;
}

static bool handleEncoderUploadTextureData()
{
    auto* encoder = getObj<ICommandEncoder>(slangReplay_getCurrentThisHandle());
    auto* dst = readObjFromStream<ITexture>();
    auto subresourceRange = readPOD<SubresourceRange>();
    auto offset = readPOD<Offset3D>();
    auto extent = readPOD<Extent3D>();
    uint32_t subresourceDataCount = slangReplay_readUInt32();

    std::vector<SubresourceData> subresources(subresourceDataCount);
    std::vector<Slang::ComPtr<ISlangBlob>> blobStorage(subresourceDataCount);
    for (uint32_t i = 0; i < subresourceDataCount; i++)
    {
        subresources[i].rowPitch = readPOD<Size>();
        subresources[i].slicePitch = readPOD<Size>();
        ISlangBlob* blob = slangReplay_readBlob();
        blobStorage[i] = blob;
        subresources[i].data = blob ? blob->getBufferPointer() : nullptr;
    }

    encoder->uploadTextureData(
        dst,
        subresourceRange,
        offset,
        extent,
        subresources.data(),
        subresourceDataCount);
    return true;
}

static bool handleEncoderUploadBufferData()
{
    auto* encoder = getObj<ICommandEncoder>(slangReplay_getCurrentThisHandle());
    auto* dst = readObjFromStream<IBuffer>();
    auto offset = readPOD<Offset>();
    auto size = readPOD<Size>();
    ISlangBlob* blob = slangReplay_readBlob();
    const void* data = blob ? blob->getBufferPointer() : nullptr;
    encoder->uploadBufferData(dst, offset, size, data);
    if (blob)
        blob->release();
    return true;
}

static bool handleEncoderClearBuffer()
{
    auto* encoder = getObj<ICommandEncoder>(slangReplay_getCurrentThisHandle());
    auto* buffer = readObjFromStream<IBuffer>();
    auto range = readPOD<BufferRange>();
    encoder->clearBuffer(buffer, range);
    return true;
}

static bool handleEncoderClearTextureFloat()
{
    auto* encoder = getObj<ICommandEncoder>(slangReplay_getCurrentThisHandle());
    auto* texture = readObjFromStream<ITexture>();
    float clearValue[4] = {};
    encoder->clearTextureFloat(texture, {}, clearValue);
    return true;
}

static bool handleEncoderClearTextureUint()
{
    auto* encoder = getObj<ICommandEncoder>(slangReplay_getCurrentThisHandle());
    auto* texture = readObjFromStream<ITexture>();
    uint32_t clearValue[4] = {};
    encoder->clearTextureUint(texture, {}, clearValue);
    return true;
}

static bool handleEncoderClearTextureSint()
{
    auto* encoder = getObj<ICommandEncoder>(slangReplay_getCurrentThisHandle());
    auto* texture = readObjFromStream<ITexture>();
    int32_t clearValue[4] = {};
    encoder->clearTextureSint(texture, {}, clearValue);
    return true;
}

static bool handleEncoderClearTextureDepthStencil()
{
    auto* encoder = getObj<ICommandEncoder>(slangReplay_getCurrentThisHandle());
    auto* texture = readObjFromStream<ITexture>();
    encoder->clearTextureDepthStencil(texture, {}, false, 0.0f, false, 0);
    return true;
}

static bool handleEncoderResolveQuery()
{
    auto* encoder = getObj<ICommandEncoder>(slangReplay_getCurrentThisHandle());
    auto* queryPool = readObjFromStream<IQueryPool>();
    auto* buffer = readObjFromStream<IBuffer>();
    encoder->resolveQuery(queryPool, 0, 0, buffer, 0);
    return true;
}

static bool handleEncoderBuildAccelerationStructure()
{
    auto* encoder = getObj<ICommandEncoder>(slangReplay_getCurrentThisHandle());
    auto* dst = readObjFromStream<IAccelerationStructure>();
    auto* src = readObjFromStream<IAccelerationStructure>();
    // Acceleration structure build involves complex nested data; stub for now.
    (void)encoder;
    (void)dst;
    (void)src;
    return true;
}

static bool handleEncoderCopyAccelerationStructure()
{
    auto* encoder = getObj<ICommandEncoder>(slangReplay_getCurrentThisHandle());
    auto* dst = readObjFromStream<IAccelerationStructure>();
    auto* src = readObjFromStream<IAccelerationStructure>();
    encoder->copyAccelerationStructure(dst, src, AccelerationStructureCopyMode::Clone);
    return true;
}

static bool handleEncoderSetBufferState()
{
    auto* encoder = getObj<ICommandEncoder>(slangReplay_getCurrentThisHandle());
    auto* buffer = readObjFromStream<IBuffer>();
    auto state = readPOD<ResourceState>();
    encoder->setBufferState(buffer, state);
    return true;
}

static bool handleEncoderSetTextureState()
{
    auto* encoder = getObj<ICommandEncoder>(slangReplay_getCurrentThisHandle());
    auto* texture = readObjFromStream<ITexture>();
    auto subresourceRange = readPOD<SubresourceRange>();
    auto state = readPOD<ResourceState>();
    encoder->setTextureState(texture, subresourceRange, state);
    return true;
}

static bool handleEncoderGlobalBarrier()
{
    auto* encoder = getObj<ICommandEncoder>(slangReplay_getCurrentThisHandle());
    encoder->globalBarrier();
    return true;
}

static bool handleEncoderWriteTimestamp()
{
    auto* encoder = getObj<ICommandEncoder>(slangReplay_getCurrentThisHandle());
    auto* queryPool = readObjFromStream<IQueryPool>();
    uint32_t queryIndex = slangReplay_readUInt32();
    encoder->writeTimestamp(queryPool, queryIndex);
    return true;
}

static bool handleEncoderFinish()
{
    auto* encoder = getObj<ICommandEncoder>(slangReplay_getCurrentThisHandle());

    ICommandBuffer* outCommandBuffer = nullptr;
    encoder->finish(&outCommandBuffer);
    mapOutputObject(outCommandBuffer);
    readReturnValue();
    return true;
}

static bool handleEncoderConvertCooperativeVectorMatrix()
{
    auto* encoder = getObj<ICommandEncoder>(slangReplay_getCurrentThisHandle());
    auto* dstBuffer = readObjFromStream<IBuffer>();
    auto* srcBuffer = readObjFromStream<IBuffer>();
    uint32_t matrixCount = slangReplay_readUInt32();

    std::vector<CooperativeVectorMatrixDesc> dstDescs(matrixCount);
    std::vector<CooperativeVectorMatrixDesc> srcDescs(matrixCount);
    for (uint32_t i = 0; i < matrixCount; i++)
    {
        dstDescs[i] = readPOD<CooperativeVectorMatrixDesc>();
        srcDescs[i] = readPOD<CooperativeVectorMatrixDesc>();
    }

    encoder->convertCooperativeVectorMatrix(
        dstBuffer,
        dstDescs.data(),
        srcBuffer,
        srcDescs.data(),
        matrixCount);
    return true;
}

static bool handleEncoderExecuteClusterOperation()
{
    auto* encoder = getObj<ICommandEncoder>(slangReplay_getCurrentThisHandle());
    auto params = readPOD<ClusterOperationParams>();

    auto* argCountBuf = readObjFromStream<IBuffer>();
    auto argCountOffset = readPOD<Offset>();
    auto* argsBuf = readObjFromStream<IBuffer>();
    auto argsOffset = readPOD<Offset>();
    auto* scratchBuf = readObjFromStream<IBuffer>();
    auto scratchOffset = readPOD<Offset>();
    auto* addressesBuf = readObjFromStream<IBuffer>();
    auto addressesOffset = readPOD<Offset>();
    auto* resultBuf = readObjFromStream<IBuffer>();
    auto resultOffset = readPOD<Offset>();
    auto* sizesBuf = readObjFromStream<IBuffer>();
    auto sizesOffset = readPOD<Offset>();

    ClusterOperationDesc desc{};
    desc.params = params;
    desc.argCountBuffer = {argCountBuf, argCountOffset};
    desc.argsBuffer = {argsBuf, argsOffset};
    desc.scratchBuffer = {scratchBuf, scratchOffset};
    desc.addressesBuffer = {addressesBuf, addressesOffset};
    desc.resultBuffer = {resultBuf, resultOffset};
    desc.sizesBuffer = {sizesBuf, sizesOffset};

    encoder->executeClusterOperation(desc);
    return true;
}

// =============================================================================
// IRenderPassEncoder handlers
// =============================================================================

static bool handleRenderBindPipeline()
{
    // Note: 'this' handle points to the encoder, but during recording the
    // pass encoder is an unowned sub-object. During replay, the pass encoder
    // is obtained from beginRenderPass and its handle is not in the handle table.
    // We use the parent command encoder's handle to locate the pass encoder.
    // For now, stub this — the architecture for pass encoder handle mapping
    // needs refinement during integration testing.
    auto handle = slangReplay_getCurrentThisHandle();
    (void)handle;
    auto* pipeline = readObjFromStream<IRenderPipeline>();
    (void)pipeline;
    uint64_t rootObjHandle = slangReplay_readHandle();
    (void)rootObjHandle;
    return true;
}

static bool handleRenderBindPipelineWithObject()
{
    auto handle = slangReplay_getCurrentThisHandle();
    (void)handle;
    auto* pipeline = readObjFromStream<IRenderPipeline>();
    auto* rootObject = readObjFromStream<IShaderObject>();
    (void)pipeline;
    (void)rootObject;
    return true;
}

static bool handleRenderSetRenderState()
{
    auto handle = slangReplay_getCurrentThisHandle();
    (void)handle;
    auto state = readPOD<RenderState>();
    (void)state;
    return true;
}

static bool handleRenderDraw()
{
    auto handle = slangReplay_getCurrentThisHandle();
    (void)handle;
    auto args = readPOD<DrawArguments>();
    (void)args;
    return true;
}

static bool handleRenderDrawIndexed()
{
    auto handle = slangReplay_getCurrentThisHandle();
    (void)handle;
    auto args = readPOD<DrawArguments>();
    (void)args;
    return true;
}

static bool handleRenderDrawIndirect()
{
    auto handle = slangReplay_getCurrentThisHandle();
    (void)handle;
    uint32_t maxDrawCount = slangReplay_readUInt32();
    auto argBuffer = readPOD<BufferOffsetPair>();
    auto countBuffer = readPOD<BufferOffsetPair>();
    (void)maxDrawCount;
    (void)argBuffer;
    (void)countBuffer;
    return true;
}

static bool handleRenderDrawIndexedIndirect()
{
    auto handle = slangReplay_getCurrentThisHandle();
    (void)handle;
    uint32_t maxDrawCount = slangReplay_readUInt32();
    auto argBuffer = readPOD<BufferOffsetPair>();
    auto countBuffer = readPOD<BufferOffsetPair>();
    (void)maxDrawCount;
    (void)argBuffer;
    (void)countBuffer;
    return true;
}

static bool handleRenderDrawMeshTasks()
{
    auto handle = slangReplay_getCurrentThisHandle();
    (void)handle;
    uint32_t x = slangReplay_readUInt32();
    uint32_t y = slangReplay_readUInt32();
    uint32_t z = slangReplay_readUInt32();
    (void)x;
    (void)y;
    (void)z;
    return true;
}

// =============================================================================
// IComputePassEncoder handlers
// =============================================================================

static bool handleComputeBindPipeline()
{
    auto handle = slangReplay_getCurrentThisHandle();
    (void)handle;
    auto* pipeline = readObjFromStream<IComputePipeline>();
    (void)pipeline;
    uint64_t rootObjHandle = slangReplay_readHandle();
    (void)rootObjHandle;
    return true;
}

static bool handleComputeBindPipelineWithObject()
{
    auto handle = slangReplay_getCurrentThisHandle();
    (void)handle;
    auto* pipeline = readObjFromStream<IComputePipeline>();
    auto* rootObject = readObjFromStream<IShaderObject>();
    (void)pipeline;
    (void)rootObject;
    return true;
}

static bool handleComputeDispatchCompute()
{
    auto handle = slangReplay_getCurrentThisHandle();
    (void)handle;
    uint32_t x = slangReplay_readUInt32();
    uint32_t y = slangReplay_readUInt32();
    uint32_t z = slangReplay_readUInt32();
    (void)x;
    (void)y;
    (void)z;
    return true;
}

static bool handleComputeDispatchComputeIndirect()
{
    auto handle = slangReplay_getCurrentThisHandle();
    (void)handle;
    auto argBuffer = readPOD<BufferOffsetPair>();
    (void)argBuffer;
    return true;
}

// =============================================================================
// IRayTracingPassEncoder handlers
// =============================================================================

static bool handleRayTracingBindPipeline()
{
    auto handle = slangReplay_getCurrentThisHandle();
    (void)handle;
    auto* pipeline = readObjFromStream<IRayTracingPipeline>();
    auto* shaderTable = readObjFromStream<IShaderTable>();
    (void)pipeline;
    (void)shaderTable;
    uint64_t rootObjHandle = slangReplay_readHandle();
    (void)rootObjHandle;
    return true;
}

static bool handleRayTracingBindPipelineWithObject()
{
    auto handle = slangReplay_getCurrentThisHandle();
    (void)handle;
    auto* pipeline = readObjFromStream<IRayTracingPipeline>();
    auto* shaderTable = readObjFromStream<IShaderTable>();
    auto* rootObject = readObjFromStream<IShaderObject>();
    (void)pipeline;
    (void)shaderTable;
    (void)rootObject;
    return true;
}

static bool handleRayTracingDispatchRays()
{
    auto handle = slangReplay_getCurrentThisHandle();
    (void)handle;
    uint32_t rayGenShaderIndex = slangReplay_readUInt32();
    uint32_t width = slangReplay_readUInt32();
    uint32_t height = slangReplay_readUInt32();
    uint32_t depth = slangReplay_readUInt32();
    (void)rayGenShaderIndex;
    (void)width;
    (void)height;
    (void)depth;
    return true;
}

// =============================================================================
// IShaderObject handlers
// =============================================================================

static bool handleShaderObjectSetData()
{
    auto* shaderObj = getObj<IShaderObject>(slangReplay_getCurrentThisHandle());
    auto offset = readPOD<ShaderOffset>();
    ISlangBlob* blob = slangReplay_readBlob();
    if (blob)
    {
        shaderObj->setData(offset, blob->getBufferPointer(), blob->getBufferSize());
        blob->release();
    }
    readReturnValue();
    return true;
}

static bool handleShaderObjectReserveData()
{
    auto* shaderObj = getObj<IShaderObject>(slangReplay_getCurrentThisHandle());
    auto offset = readPOD<ShaderOffset>();
    auto size = readPOD<size_t>();
    void* outData = nullptr;
    shaderObj->reserveData(offset, size, &outData);
    readReturnValue();
    return true;
}

static bool handleShaderObjectSetObject()
{
    auto* shaderObj = getObj<IShaderObject>(slangReplay_getCurrentThisHandle());
    auto offset = readPOD<ShaderOffset>();
    auto* childObj = readObjFromStream<IShaderObject>();
    shaderObj->setObject(offset, childObj);
    readReturnValue();
    return true;
}

static bool handleShaderObjectSetBinding()
{
    auto* shaderObj = getObj<IShaderObject>(slangReplay_getCurrentThisHandle());
    auto offset = readPOD<ShaderOffset>();
    auto binding = readPOD<Binding>();
    // The binding's resource pointers are raw recorded values; we'd need to
    // resolve them from handles, but they were recorded as POD (not handles).
    // This is a known limitation — Binding contains IResource* pointers
    // that were serialized as part of the POD struct and need special handling.
    shaderObj->setBinding(offset, binding);
    readReturnValue();
    return true;
}

static bool handleShaderObjectSetDescriptorHandle()
{
    auto* shaderObj = getObj<IShaderObject>(slangReplay_getCurrentThisHandle());
    auto offset = readPOD<ShaderOffset>();
    auto handle = readPOD<DescriptorHandle>();
    shaderObj->setDescriptorHandle(offset, handle);
    readReturnValue();
    return true;
}

static bool handleShaderObjectSetSpecializationArgs()
{
    auto* shaderObj = getObj<IShaderObject>(slangReplay_getCurrentThisHandle());
    auto offset = readPOD<ShaderOffset>();
    uint32_t count = slangReplay_readUInt32();
    std::vector<slang::SpecializationArg> args(count);
    for (uint32_t i = 0; i < count; i++)
        args[i] = readPOD<slang::SpecializationArg>();
    shaderObj->setSpecializationArgs(offset, args.data(), count);
    readReturnValue();
    return true;
}

static bool handleShaderObjectSetConstantBufferOverride()
{
    auto* shaderObj = getObj<IShaderObject>(slangReplay_getCurrentThisHandle());
    auto* buffer = readObjFromStream<IBuffer>();
    shaderObj->setConstantBufferOverride(buffer);
    readReturnValue();
    return true;
}

static bool handleShaderObjectFinalize()
{
    auto* shaderObj = getObj<IShaderObject>(slangReplay_getCurrentThisHandle());
    shaderObj->finalize();
    readReturnValue();
    return true;
}

// =============================================================================
// External handler dispatch
// =============================================================================

static bool rhiDispatchCall(const char* signature)
{
    auto& map = getHandlerMap();
    auto it = map.find(signature);
    if (it == map.end())
        return false;
    return it->second();
}

// =============================================================================
// Registration
// =============================================================================

void registerHandlers()
{
    // Static/free functions
    reg("rhi::createDevice", handleCreateDevice);

    // IDevice
    reg("IDevice::createBuffer", handleDeviceCreateBuffer);
    reg("IDevice::createBufferFromNativeHandle", handleDeviceCreateBufferFromNativeHandle);
    reg("IDevice::createBufferFromSharedHandle", handleDeviceCreateBufferFromSharedHandle);
    reg("IDevice::createTexture", handleDeviceCreateTexture);
    reg("IDevice::createTextureFromNativeHandle", handleDeviceCreateTextureFromNativeHandle);
    reg("IDevice::createTextureFromSharedHandle", handleDeviceCreateTextureFromSharedHandle);
    reg("IDevice::mapBuffer", handleDeviceMapBuffer);
    reg("IDevice::unmapBuffer", handleDeviceUnmapBuffer);
    reg("IDevice::createSampler", handleDeviceCreateSampler);
    reg("IDevice::createTextureView", handleDeviceCreateTextureView);
    reg("IDevice::createAccelerationStructure", handleDeviceCreateAccelerationStructure);
    reg("IDevice::createSurface", handleDeviceCreateSurface);
    reg("IDevice::createInputLayout", handleDeviceCreateInputLayout);
    reg("IDevice::getQueue", handleDeviceGetQueue);
    reg("IDevice::createShaderObject", handleDeviceCreateShaderObject);
    reg("IDevice::createShaderObjectFromTypeLayout", handleDeviceCreateShaderObjectFromTypeLayout);
    reg("IDevice::createRootShaderObject", handleDeviceCreateRootShaderObject);
    reg("IDevice::createShaderProgram", handleDeviceCreateShaderProgram);
    reg("IDevice::createRenderPipeline", handleDeviceCreateRenderPipeline);
    reg("IDevice::createComputePipeline", handleDeviceCreateComputePipeline);
    reg("IDevice::createRayTracingPipeline", handleDeviceCreateRayTracingPipeline);
    reg("IDevice::readTexture", handleDeviceReadTexture);
    reg("IDevice::readTextureBlob", handleDeviceReadTextureBlob);
    reg("IDevice::readBuffer", handleDeviceReadBuffer);
    reg("IDevice::readBufferBlob", handleDeviceReadBufferBlob);
    reg("IDevice::createQueryPool", handleDeviceCreateQueryPool);
    reg("IDevice::createFence", handleDeviceCreateFence);
    reg("IDevice::waitForFences", handleDeviceWaitForFences);
    reg("IDevice::createHeap", handleDeviceCreateHeap);
    reg("IDevice::convertCooperativeVectorMatrix", handleDeviceConvertCooperativeVectorMatrix);
    reg("IDevice::createShaderTable", handleDeviceCreateShaderTable);

    // ICommandQueue
    reg("ICommandQueue::createCommandEncoder", handleQueueCreateCommandEncoder);
    reg("ICommandQueue::submit", handleQueueSubmit);
    reg("ICommandQueue::waitOnHost", handleQueueWaitOnHost);

    // ICommandEncoder
    reg("ICommandEncoder::beginRenderPass", handleEncoderBeginRenderPass);
    reg("ICommandEncoder::beginComputePass", handleEncoderBeginComputePass);
    reg("ICommandEncoder::beginRayTracingPass", handleEncoderBeginRayTracingPass);
    reg("ICommandEncoder::copyBuffer", handleEncoderCopyBuffer);
    reg("ICommandEncoder::copyTexture", handleEncoderCopyTexture);
    reg("ICommandEncoder::copyTextureToBuffer", handleEncoderCopyTextureToBuffer);
    reg("ICommandEncoder::copyBufferToTexture", handleEncoderCopyBufferToTexture);
    reg("ICommandEncoder::uploadTextureData", handleEncoderUploadTextureData);
    reg("ICommandEncoder::uploadBufferData", handleEncoderUploadBufferData);
    reg("ICommandEncoder::clearBuffer", handleEncoderClearBuffer);
    reg("ICommandEncoder::clearTextureFloat", handleEncoderClearTextureFloat);
    reg("ICommandEncoder::clearTextureUint", handleEncoderClearTextureUint);
    reg("ICommandEncoder::clearTextureSint", handleEncoderClearTextureSint);
    reg("ICommandEncoder::clearTextureDepthStencil", handleEncoderClearTextureDepthStencil);
    reg("ICommandEncoder::resolveQuery", handleEncoderResolveQuery);
    reg("ICommandEncoder::buildAccelerationStructure", handleEncoderBuildAccelerationStructure);
    reg("ICommandEncoder::copyAccelerationStructure", handleEncoderCopyAccelerationStructure);
    reg("ICommandEncoder::setBufferState", handleEncoderSetBufferState);
    reg("ICommandEncoder::setTextureState", handleEncoderSetTextureState);
    reg("ICommandEncoder::globalBarrier", handleEncoderGlobalBarrier);
    reg("ICommandEncoder::writeTimestamp", handleEncoderWriteTimestamp);
    reg("ICommandEncoder::finish", handleEncoderFinish);
    reg("ICommandEncoder::convertCooperativeVectorMatrix", handleEncoderConvertCooperativeVectorMatrix);
    reg("ICommandEncoder::executeClusterOperation", handleEncoderExecuteClusterOperation);

    // IRenderPassEncoder
    reg("IRenderPassEncoder::bindPipeline", handleRenderBindPipeline);
    reg("IRenderPassEncoder::bindPipelineWithObject", handleRenderBindPipelineWithObject);
    reg("IRenderPassEncoder::setRenderState", handleRenderSetRenderState);
    reg("IRenderPassEncoder::draw", handleRenderDraw);
    reg("IRenderPassEncoder::drawIndexed", handleRenderDrawIndexed);
    reg("IRenderPassEncoder::drawIndirect", handleRenderDrawIndirect);
    reg("IRenderPassEncoder::drawIndexedIndirect", handleRenderDrawIndexedIndirect);
    reg("IRenderPassEncoder::drawMeshTasks", handleRenderDrawMeshTasks);

    // IComputePassEncoder
    reg("IComputePassEncoder::bindPipeline", handleComputeBindPipeline);
    reg("IComputePassEncoder::bindPipelineWithObject", handleComputeBindPipelineWithObject);
    reg("IComputePassEncoder::dispatchCompute", handleComputeDispatchCompute);
    reg("IComputePassEncoder::dispatchComputeIndirect", handleComputeDispatchComputeIndirect);

    // IRayTracingPassEncoder
    reg("IRayTracingPassEncoder::bindPipeline", handleRayTracingBindPipeline);
    reg("IRayTracingPassEncoder::bindPipelineWithObject", handleRayTracingBindPipelineWithObject);
    reg("IRayTracingPassEncoder::dispatchRays", handleRayTracingDispatchRays);

    // IShaderObject
    reg("IShaderObject::setData", handleShaderObjectSetData);
    reg("IShaderObject::reserveData", handleShaderObjectReserveData);
    reg("IShaderObject::setObject", handleShaderObjectSetObject);
    reg("IShaderObject::setBinding", handleShaderObjectSetBinding);
    reg("IShaderObject::setDescriptorHandle", handleShaderObjectSetDescriptorHandle);
    reg("IShaderObject::setSpecializationArgs", handleShaderObjectSetSpecializationArgs);
    reg("IShaderObject::setConstantBufferOverride", handleShaderObjectSetConstantBufferOverride);
    reg("IShaderObject::finalize", handleShaderObjectFinalize);

    // Set the external handler
    slangReplay_setExternalHandler(rhiDispatchCall);
}

} // namespace rhi::replay
