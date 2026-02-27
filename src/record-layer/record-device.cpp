#include "record-device.h"
#include "record-command-buffer.h"
#include "record-command-queue.h"
#include "record-fence.h"
#include "record-heap.h"
#include "record-pipeline.h"
#include "record-query.h"
#include "record-resource.h"
#include "record-shader-object.h"
#include "record-surface.h"

#include "reference.h"
#include "resource-desc-utils.h"
#include "core/short_vector.h"

namespace rhi::record {

template<typename RecordT, typename InterfaceT>
static void wrapOutput(RefPtr<RecordT>& wrapped, InterfaceT** outObj)
{
    if (outObj)
    {
        if (wrapped->baseObject)
        {
            wrapped->registerSelf();
            returnComPtr(outObj, wrapped);
        }
        else
        {
            *outObj = nullptr;
        }
    }
}

// =============================================================================
// Query / info methods (passthrough, no recording needed)
// =============================================================================

Result RecordDevice::getNativeDeviceHandles(DeviceNativeHandles* outHandles)
{
    return baseObject->getNativeDeviceHandles(outHandles);
}

Result RecordDevice::getFeatures(uint32_t* outFeatureCount, Feature* outFeatures)
{
    return baseObject->getFeatures(outFeatureCount, outFeatures);
}

bool RecordDevice::hasFeature(Feature feature)
{
    return baseObject->hasFeature(feature);
}

bool RecordDevice::hasFeature(const char* feature)
{
    return baseObject->hasFeature(feature);
}

Result RecordDevice::getCapabilities(uint32_t* outCapabilityCount, Capability* outCapabilities)
{
    return baseObject->getCapabilities(outCapabilityCount, outCapabilities);
}

bool RecordDevice::hasCapability(Capability capability)
{
    return baseObject->hasCapability(capability);
}

bool RecordDevice::hasCapability(const char* capability)
{
    return baseObject->hasCapability(capability);
}

Result RecordDevice::getFormatSupport(Format format, FormatSupport* outFormatSupport)
{
    return baseObject->getFormatSupport(format, outFormatSupport);
}

Result RecordDevice::getSlangSession(slang::ISession** outSlangSession)
{
    return baseObject->getSlangSession(outSlangSession);
}

const DeviceInfo& RecordDevice::getInfo() const
{
    return baseObject->getInfo();
}

Result RecordDevice::getTextureAllocationInfo(
    const TextureDesc& desc,
    size_t* outSize,
    size_t* outAlignment)
{
    return baseObject->getTextureAllocationInfo(desc, outSize, outAlignment);
}

Result RecordDevice::getTextureRowAlignment(Format format, size_t* outAlignment)
{
    return baseObject->getTextureRowAlignment(format, outAlignment);
}

Result RecordDevice::getCooperativeVectorProperties(
    CooperativeVectorProperties* properties,
    uint32_t* propertiesCount)
{
    return baseObject->getCooperativeVectorProperties(properties, propertiesCount);
}

Result RecordDevice::getCooperativeVectorMatrixSize(
    uint32_t rowCount,
    uint32_t colCount,
    CooperativeVectorComponentType componentType,
    CooperativeVectorMatrixLayout layout,
    size_t rowColumnStride,
    size_t* outSize)
{
    return baseObject->getCooperativeVectorMatrixSize(
        rowCount,
        colCount,
        componentType,
        layout,
        rowColumnStride,
        outSize);
}

Result RecordDevice::reportHeaps(HeapReport* heapReports, uint32_t* heapCount)
{
    return baseObject->reportHeaps(heapReports, heapCount);
}

// =============================================================================
// Resource creation methods (recorded)
// =============================================================================

Result RecordDevice::createBuffer(
    const BufferDesc& desc,
    const void* initData,
    IBuffer** outBuffer)
{
    RHI_RECORD_CALL("IDevice::createBuffer");
    RHI_RECORD_INPUT_DESC(desc);
    RHI_RECORD_INPUT_BLOB(initData, desc.size);

    RefPtr<RecordBuffer> wrapped = new RecordBuffer();
    auto result = baseObject->createBuffer(desc, initData, wrapped->baseObject.writeRef());
    wrapOutput(wrapped, outBuffer);
    RHI_RECORD_OBJECT_OUTPUT(outBuffer);
    RHI_RECORD_RETURN(result);
}

Result RecordDevice::createBufferFromNativeHandle(
    NativeHandle handle,
    const BufferDesc& desc,
    IBuffer** outBuffer)
{
    RHI_RECORD_CALL("IDevice::createBufferFromNativeHandle");
    RHI_RECORD_INPUT_POD(handle);
    RHI_RECORD_INPUT_DESC(desc);

    RefPtr<RecordBuffer> wrapped = new RecordBuffer();
    auto result = baseObject->createBufferFromNativeHandle(handle, desc, wrapped->baseObject.writeRef());
    wrapOutput(wrapped, outBuffer);
    RHI_RECORD_OBJECT_OUTPUT(outBuffer);
    RHI_RECORD_RETURN(result);
}

Result RecordDevice::createBufferFromSharedHandle(
    NativeHandle handle,
    const BufferDesc& desc,
    IBuffer** outBuffer)
{
    RHI_RECORD_CALL("IDevice::createBufferFromSharedHandle");
    RHI_RECORD_INPUT_POD(handle);
    RHI_RECORD_INPUT_DESC(desc);

    RefPtr<RecordBuffer> wrapped = new RecordBuffer();
    auto result = baseObject->createBufferFromSharedHandle(handle, desc, wrapped->baseObject.writeRef());
    wrapOutput(wrapped, outBuffer);
    RHI_RECORD_OBJECT_OUTPUT(outBuffer);
    RHI_RECORD_RETURN(result);
}

Result RecordDevice::createTexture(
    const TextureDesc& desc,
    const SubresourceData* initData,
    ITexture** outTexture)
{
    RHI_RECORD_CALL("IDevice::createTexture");
    RHI_RECORD_INPUT_DESC(desc);

    if (initData)
    {
        uint32_t layerCount = desc.getLayerCount();
        uint32_t subresourceCount = desc.mipCount * layerCount;
        RHI_RECORD_INPUT_UINT32(subresourceCount);
        uint32_t idx = 0;
        for (uint32_t layer = 0; layer < layerCount; layer++)
        {
            for (uint32_t mip = 0; mip < desc.mipCount; mip++)
            {
                uint32_t depth = calcMipSize(desc.size.depth, mip);
                size_t dataSize = initData[idx].slicePitch * depth;
                RHI_RECORD_INPUT_POD(initData[idx].rowPitch);
                RHI_RECORD_INPUT_POD(initData[idx].slicePitch);
                RHI_RECORD_INPUT_BLOB(initData[idx].data, dataSize);
                idx++;
            }
        }
    }
    else
    {
        uint32_t zero = 0;
        RHI_RECORD_INPUT_UINT32(zero);
    }

    RefPtr<RecordTexture> wrapped = new RecordTexture();
    auto result = baseObject->createTexture(desc, initData, wrapped->baseObject.writeRef());
    wrapOutput(wrapped, outTexture);
    RHI_RECORD_OBJECT_OUTPUT(outTexture);
    RHI_RECORD_RETURN(result);
}

Result RecordDevice::createTextureFromNativeHandle(
    NativeHandle handle,
    const TextureDesc& desc,
    ITexture** outTexture)
{
    RHI_RECORD_CALL("IDevice::createTextureFromNativeHandle");
    RHI_RECORD_INPUT_POD(handle);
    RHI_RECORD_INPUT_DESC(desc);

    RefPtr<RecordTexture> wrapped = new RecordTexture();
    auto result = baseObject->createTextureFromNativeHandle(handle, desc, wrapped->baseObject.writeRef());
    wrapOutput(wrapped, outTexture);
    RHI_RECORD_OBJECT_OUTPUT(outTexture);
    RHI_RECORD_RETURN(result);
}

Result RecordDevice::createTextureFromSharedHandle(
    NativeHandle handle,
    const TextureDesc& desc,
    const Size size,
    ITexture** outTexture)
{
    RHI_RECORD_CALL("IDevice::createTextureFromSharedHandle");
    RHI_RECORD_INPUT_POD(handle);
    RHI_RECORD_INPUT_DESC(desc);
    RHI_RECORD_INPUT_POD(size);

    RefPtr<RecordTexture> wrapped = new RecordTexture();
    auto result = baseObject->createTextureFromSharedHandle(handle, desc, size, wrapped->baseObject.writeRef());
    wrapOutput(wrapped, outTexture);
    RHI_RECORD_OBJECT_OUTPUT(outTexture);
    RHI_RECORD_RETURN(result);
}

Result RecordDevice::mapBuffer(IBuffer* buffer, CpuAccessMode mode, void** outData)
{
    RHI_RECORD_CALL("IDevice::mapBuffer");
    RHI_RECORD_OBJECT_INPUT(buffer);
    RHI_RECORD_INPUT_POD(mode);

    auto result = baseObject->mapBuffer(getInnerObj(buffer), mode, outData);
    if (SLANG_SUCCEEDED(result) && outData && *outData)
    {
        auto* bufProxy = checked_cast<RecordObject<IBuffer>*>(buffer);
        Size bufSize = bufProxy->baseObject->getDesc().size;
        m_mappedBuffers[buffer] = {*outData, bufSize, mode};
    }
    RHI_RECORD_RETURN(result);
}

Result RecordDevice::unmapBuffer(IBuffer* buffer)
{
    RHI_RECORD_CALL("IDevice::unmapBuffer");
    RHI_RECORD_OBJECT_INPUT(buffer);

    auto it = m_mappedBuffers.find(buffer);
    if (it != m_mappedBuffers.end())
    {
        if (it->second.mode == CpuAccessMode::Write)
            RHI_RECORD_INPUT_BLOB(it->second.data, it->second.size);
        m_mappedBuffers.erase(it);
    }

    auto result = baseObject->unmapBuffer(getInnerObj(buffer));
    RHI_RECORD_RETURN(result);
}

Result RecordDevice::createSampler(const SamplerDesc& desc, ISampler** outSampler)
{
    RHI_RECORD_CALL("IDevice::createSampler");
    RHI_RECORD_INPUT_DESC(desc);

    RefPtr<RecordSampler> wrapped = new RecordSampler();
    auto result = baseObject->createSampler(desc, wrapped->baseObject.writeRef());
    wrapOutput(wrapped, outSampler);
    RHI_RECORD_OBJECT_OUTPUT(outSampler);
    RHI_RECORD_RETURN(result);
}

Result RecordDevice::createTextureView(
    ITexture* texture,
    const TextureViewDesc& desc,
    ITextureView** outView)
{
    RHI_RECORD_CALL("IDevice::createTextureView");
    RHI_RECORD_OBJECT_INPUT(texture);
    RHI_RECORD_INPUT_DESC(desc);

    RefPtr<RecordTextureView> wrapped = new RecordTextureView();
    auto result = baseObject->createTextureView(getInnerObj(texture), desc, wrapped->baseObject.writeRef());
    wrapOutput(wrapped, outView);
    RHI_RECORD_OBJECT_OUTPUT(outView);
    RHI_RECORD_RETURN(result);
}

// =============================================================================
// Acceleration structure
// =============================================================================

Result RecordDevice::getAccelerationStructureSizes(
    const AccelerationStructureBuildDesc& desc,
    AccelerationStructureSizes* outSizes)
{
    return baseObject->getAccelerationStructureSizes(desc, outSizes);
}

Result RecordDevice::getClusterOperationSizes(
    const ClusterOperationParams& params,
    ClusterOperationSizes* outSizes)
{
    return baseObject->getClusterOperationSizes(params, outSizes);
}

Result RecordDevice::createAccelerationStructure(
    const AccelerationStructureDesc& desc,
    IAccelerationStructure** outAccelerationStructure)
{
    RHI_RECORD_CALL("IDevice::createAccelerationStructure");
    RHI_RECORD_INPUT_DESC(desc);

    RefPtr<RecordAccelerationStructure> wrapped = new RecordAccelerationStructure();
    auto result = baseObject->createAccelerationStructure(desc, wrapped->baseObject.writeRef());
    wrapOutput(wrapped, outAccelerationStructure);
    RHI_RECORD_OBJECT_OUTPUT(outAccelerationStructure);
    RHI_RECORD_RETURN(result);
}

// =============================================================================
// Surface, input layout, queue
// =============================================================================

Result RecordDevice::createSurface(WindowHandle windowHandle, ISurface** outSurface)
{
    RHI_RECORD_CALL("IDevice::createSurface");

    RefPtr<RecordSurface> wrapped = new RecordSurface();
    auto result = baseObject->createSurface(windowHandle, wrapped->baseObject.writeRef());
    wrapOutput(wrapped, outSurface);
    RHI_RECORD_OBJECT_OUTPUT(outSurface);
    RHI_RECORD_RETURN(result);
}

Result RecordDevice::createInputLayout(const InputLayoutDesc& desc, IInputLayout** outLayout)
{
    RHI_RECORD_CALL("IDevice::createInputLayout");
    RHI_RECORD_INPUT_DESC(desc);
    for (uint32_t i = 0; i < desc.inputElementCount; i++)
    {
        slangRecord_recordString(SLANG_RECORD_FLAG_INPUT, desc.inputElements[i].semanticName);
        RHI_RECORD_INPUT_UINT32(desc.inputElements[i].semanticIndex);
        RHI_RECORD_INPUT_POD(desc.inputElements[i].format);
        RHI_RECORD_INPUT_UINT32(desc.inputElements[i].offset);
        RHI_RECORD_INPUT_UINT32(desc.inputElements[i].bufferSlotIndex);
    }
    RHI_RECORD_INPUT_POD_ARRAY(desc.vertexStreams, desc.vertexStreamCount);

    RefPtr<RecordInputLayout> wrapped = new RecordInputLayout();
    auto result = baseObject->createInputLayout(desc, wrapped->baseObject.writeRef());
    wrapOutput(wrapped, outLayout);
    RHI_RECORD_OBJECT_OUTPUT(outLayout);
    RHI_RECORD_RETURN(result);
}

Result RecordDevice::getQueue(QueueType type, ICommandQueue** outQueue)
{
    RHI_RECORD_CALL("IDevice::getQueue");
    RHI_RECORD_INPUT_POD(type);

    auto it = m_queueCache.find(type);
    if (it != m_queueCache.end())
    {
        *outQueue = it->second;
        it->second->addRef();
        RHI_RECORD_OBJECT_OUTPUT(outQueue);
        return SLANG_OK;
    }

    RefPtr<RecordCommandQueue> wrapped = new RecordCommandQueue();
    auto result = baseObject->getQueue(type, wrapped->baseObject.writeRef());
    wrapOutput(wrapped, outQueue);
    if (SLANG_SUCCEEDED(result) && *outQueue)
        m_queueCache[type] = checked_cast<RecordCommandQueue*>(*outQueue);
    RHI_RECORD_OBJECT_OUTPUT(outQueue);
    RHI_RECORD_RETURN(result);
}

// =============================================================================
// Shader objects
// =============================================================================

Result RecordDevice::createShaderObject(
    slang::ISession* session,
    slang::TypeReflection* type,
    ShaderObjectContainerType container,
    IShaderObject** outObject)
{
    RHI_RECORD_CALL("IDevice::createShaderObject");

    RefPtr<RecordShaderObject> wrapped = new RecordShaderObject();
    auto result = baseObject->createShaderObject(session, type, container, wrapped->baseObject.writeRef());
    wrapOutput(wrapped, outObject);
    RHI_RECORD_OBJECT_OUTPUT(outObject);
    RHI_RECORD_RETURN(result);
}

Result RecordDevice::createShaderObjectFromTypeLayout(
    slang::TypeLayoutReflection* typeLayout,
    IShaderObject** outObject)
{
    RHI_RECORD_CALL("IDevice::createShaderObjectFromTypeLayout");

    RefPtr<RecordShaderObject> wrapped = new RecordShaderObject();
    auto result = baseObject->createShaderObjectFromTypeLayout(typeLayout, wrapped->baseObject.writeRef());
    wrapOutput(wrapped, outObject);
    RHI_RECORD_OBJECT_OUTPUT(outObject);
    RHI_RECORD_RETURN(result);
}

Result RecordDevice::createRootShaderObject(
    IShaderProgram* program,
    IShaderObject** outObject)
{
    RHI_RECORD_CALL("IDevice::createRootShaderObject");
    RHI_RECORD_OBJECT_INPUT(program);

    RefPtr<RecordShaderObject> wrapped = new RecordShaderObject();
    auto result = baseObject->createRootShaderObject(getInnerObj(program), wrapped->baseObject.writeRef());
    wrapOutput(wrapped, outObject);
    RHI_RECORD_OBJECT_OUTPUT(outObject);
    RHI_RECORD_RETURN(result);
}

// =============================================================================
// Shader program & pipeline creation
// =============================================================================

Result RecordDevice::createShaderProgram(
    const ShaderProgramDesc& desc,
    IShaderProgram** outProgram,
    ISlangBlob** outDiagnostics)
{
    RHI_RECORD_CALL("IDevice::createShaderProgram");
    RHI_RECORD_INPUT_DESC(desc);
    RHI_RECORD_OBJECT_INPUT(desc.slangGlobalScope);
    RHI_RECORD_INPUT_UINT32(desc.slangEntryPointCount);
    for (uint32_t i = 0; i < desc.slangEntryPointCount; i++)
        RHI_RECORD_OBJECT_INPUT(desc.slangEntryPoints[i]);

    RefPtr<RecordShaderProgram> wrapped = new RecordShaderProgram();
    auto result = baseObject->createShaderProgram(desc, wrapped->baseObject.writeRef(), outDiagnostics);
    wrapOutput(wrapped, outProgram);
    RHI_RECORD_OBJECT_OUTPUT(outProgram);
    RHI_RECORD_RETURN(result);
}

Result RecordDevice::createRenderPipeline(
    const RenderPipelineDesc& desc,
    IRenderPipeline** outPipeline)
{
    RHI_RECORD_CALL("IDevice::createRenderPipeline");
    RHI_RECORD_INPUT_DESC(desc);
    RHI_RECORD_OBJECT_INPUT(desc.program);
    RHI_RECORD_OBJECT_INPUT(desc.inputLayout);
    RHI_RECORD_INPUT_POD_ARRAY(desc.targets, desc.targetCount);
    if (desc.label)
        slangRecord_recordString(SLANG_RECORD_FLAG_INPUT, desc.label);

    RenderPipelineDesc innerDesc = desc;
    innerDesc.program = getInnerObj(desc.program);
    innerDesc.inputLayout = getInnerObj(desc.inputLayout);
    RefPtr<RecordRenderPipeline> wrapped = new RecordRenderPipeline();
    auto result = baseObject->createRenderPipeline(innerDesc, wrapped->baseObject.writeRef());
    wrapOutput(wrapped, outPipeline);
    RHI_RECORD_OBJECT_OUTPUT(outPipeline);
    RHI_RECORD_RETURN(result);
}

Result RecordDevice::createComputePipeline(
    const ComputePipelineDesc& desc,
    IComputePipeline** outPipeline)
{
    RHI_RECORD_CALL("IDevice::createComputePipeline");
    RHI_RECORD_INPUT_DESC(desc);

    ComputePipelineDesc innerDesc = desc;
    innerDesc.program = getInnerObj(desc.program);
    RefPtr<RecordComputePipeline> wrapped = new RecordComputePipeline();
    auto result = baseObject->createComputePipeline(innerDesc, wrapped->baseObject.writeRef());
    wrapOutput(wrapped, outPipeline);
    RHI_RECORD_OBJECT_OUTPUT(outPipeline);
    RHI_RECORD_RETURN(result);
}

Result RecordDevice::createRayTracingPipeline(
    const RayTracingPipelineDesc& desc,
    IRayTracingPipeline** outPipeline)
{
    RHI_RECORD_CALL("IDevice::createRayTracingPipeline");
    RHI_RECORD_INPUT_DESC(desc);
    RHI_RECORD_OBJECT_INPUT(desc.program);
    RHI_RECORD_INPUT_UINT32(desc.hitGroupCount);
    for (uint32_t i = 0; i < desc.hitGroupCount; i++)
    {
        slangRecord_recordString(SLANG_RECORD_FLAG_INPUT, desc.hitGroups[i].hitGroupName);
        slangRecord_recordString(SLANG_RECORD_FLAG_INPUT, desc.hitGroups[i].closestHitEntryPoint);
        slangRecord_recordString(SLANG_RECORD_FLAG_INPUT, desc.hitGroups[i].anyHitEntryPoint);
        slangRecord_recordString(SLANG_RECORD_FLAG_INPUT, desc.hitGroups[i].intersectionEntryPoint);
    }
    if (desc.label)
        slangRecord_recordString(SLANG_RECORD_FLAG_INPUT, desc.label);

    RayTracingPipelineDesc innerDesc = desc;
    innerDesc.program = getInnerObj(desc.program);
    RefPtr<RecordRayTracingPipeline> wrapped = new RecordRayTracingPipeline();
    auto result = baseObject->createRayTracingPipeline(innerDesc, wrapped->baseObject.writeRef());
    wrapOutput(wrapped, outPipeline);
    RHI_RECORD_OBJECT_OUTPUT(outPipeline);
    RHI_RECORD_RETURN(result);
}

Result RecordDevice::getCompilationReportList(ISlangBlob** outReportListBlob)
{
    return baseObject->getCompilationReportList(outReportListBlob);
}

// =============================================================================
// Read-back methods
// =============================================================================

Result RecordDevice::readTexture(
    ITexture* texture,
    uint32_t layer,
    uint32_t mip,
    const SubresourceLayout& layout,
    void* outData)
{
    RHI_RECORD_CALL("IDevice::readTexture");
    RHI_RECORD_OBJECT_INPUT(texture);
    RHI_RECORD_INPUT_UINT32(layer);
    RHI_RECORD_INPUT_UINT32(mip);
    return baseObject->readTexture(getInnerObj(texture), layer, mip, layout, outData);
}

Result RecordDevice::readTexture(
    ITexture* texture,
    uint32_t layer,
    uint32_t mip,
    ISlangBlob** outBlob,
    SubresourceLayout* outLayout)
{
    RHI_RECORD_CALL("IDevice::readTextureBlob");
    RHI_RECORD_OBJECT_INPUT(texture);
    RHI_RECORD_INPUT_UINT32(layer);
    RHI_RECORD_INPUT_UINT32(mip);
    return baseObject->readTexture(getInnerObj(texture), layer, mip, outBlob, outLayout);
}

Result RecordDevice::readBuffer(
    IBuffer* buffer,
    Offset offset,
    Size size,
    void* outData)
{
    RHI_RECORD_CALL("IDevice::readBuffer");
    RHI_RECORD_OBJECT_INPUT(buffer);
    RHI_RECORD_INPUT_POD(offset);
    RHI_RECORD_INPUT_POD(size);
    return baseObject->readBuffer(getInnerObj(buffer), offset, size, outData);
}

Result RecordDevice::readBuffer(
    IBuffer* buffer,
    Offset offset,
    Size size,
    ISlangBlob** outBlob)
{
    RHI_RECORD_CALL("IDevice::readBufferBlob");
    RHI_RECORD_OBJECT_INPUT(buffer);
    RHI_RECORD_INPUT_POD(offset);
    RHI_RECORD_INPUT_POD(size);
    return baseObject->readBuffer(getInnerObj(buffer), offset, size, outBlob);
}

// =============================================================================
// Fence, query pool, heap
// =============================================================================

Result RecordDevice::createQueryPool(const QueryPoolDesc& desc, IQueryPool** outPool)
{
    RHI_RECORD_CALL("IDevice::createQueryPool");
    RHI_RECORD_INPUT_DESC(desc);

    RefPtr<RecordQueryPool> wrapped = new RecordQueryPool();
    auto result = baseObject->createQueryPool(desc, wrapped->baseObject.writeRef());
    wrapOutput(wrapped, outPool);
    RHI_RECORD_OBJECT_OUTPUT(outPool);
    RHI_RECORD_RETURN(result);
}

Result RecordDevice::createFence(const FenceDesc& desc, IFence** outFence)
{
    RHI_RECORD_CALL("IDevice::createFence");
    RHI_RECORD_INPUT_DESC(desc);

    RefPtr<RecordFence> wrapped = new RecordFence();
    auto result = baseObject->createFence(desc, wrapped->baseObject.writeRef());
    wrapOutput(wrapped, outFence);
    RHI_RECORD_OBJECT_OUTPUT(outFence);
    RHI_RECORD_RETURN(result);
}

Result RecordDevice::waitForFences(
    uint32_t fenceCount,
    IFence** fences,
    const uint64_t* fenceValues,
    bool waitForAll,
    uint64_t timeout)
{
    RHI_RECORD_CALL("IDevice::waitForFences");
    RHI_RECORD_INPUT_UINT32(fenceCount);
    RHI_RECORD_INPUT_BOOL(waitForAll);
    RHI_RECORD_INPUT_UINT64(timeout);

    // Unwrap fence proxies for the real call
    short_vector<IFence*> innerFences;
    for (uint32_t i = 0; i < fenceCount; i++)
        innerFences.push_back(getInnerObj(fences[i]));

    auto result = baseObject->waitForFences(
        fenceCount,
        innerFences.data(),
        fenceValues,
        waitForAll,
        timeout);
    RHI_RECORD_RETURN(result);
}

Result RecordDevice::createHeap(const HeapDesc& desc, IHeap** outHeap)
{
    RHI_RECORD_CALL("IDevice::createHeap");
    RHI_RECORD_INPUT_POD(desc);

    RefPtr<RecordHeap> wrapped = new RecordHeap();
    auto result = baseObject->createHeap(desc, wrapped->baseObject.writeRef());
    wrapOutput(wrapped, outHeap);
    RHI_RECORD_OBJECT_OUTPUT(outHeap);
    RHI_RECORD_RETURN(result);
}

// =============================================================================
// Misc
// =============================================================================

Result RecordDevice::convertCooperativeVectorMatrix(
    void* dstBuffer,
    size_t dstBufferSize,
    const CooperativeVectorMatrixDesc* dstDescs,
    const void* srcBuffer,
    size_t srcBufferSize,
    const CooperativeVectorMatrixDesc* srcDescs,
    uint32_t matrixCount)
{
    RHI_RECORD_CALL("IDevice::convertCooperativeVectorMatrix");
    RHI_RECORD_INPUT_POD(dstBufferSize);
    RHI_RECORD_INPUT_POD(srcBufferSize);
    RHI_RECORD_INPUT_UINT32(matrixCount);
    RHI_RECORD_INPUT_BLOB(srcBuffer, srcBufferSize);
    for (uint32_t i = 0; i < matrixCount; i++)
    {
        RHI_RECORD_INPUT_POD(dstDescs[i]);
        RHI_RECORD_INPUT_POD(srcDescs[i]);
    }

    auto result = baseObject->convertCooperativeVectorMatrix(
        dstBuffer,
        dstBufferSize,
        dstDescs,
        srcBuffer,
        srcBufferSize,
        srcDescs,
        matrixCount);
    RHI_RECORD_RETURN(result);
}

Result RecordDevice::createShaderTable(const ShaderTableDesc& desc, IShaderTable** outTable)
{
    RHI_RECORD_CALL("IDevice::createShaderTable");
    RHI_RECORD_INPUT_DESC(desc);
    RHI_RECORD_OBJECT_INPUT(desc.program);

    RHI_RECORD_INPUT_STRING_ARRAY(desc.rayGenShaderEntryPointNames, desc.rayGenShaderCount);
    RHI_RECORD_INPUT_POD_ARRAY(desc.rayGenShaderRecordOverwrites, desc.rayGenShaderCount);

    RHI_RECORD_INPUT_STRING_ARRAY(desc.missShaderEntryPointNames, desc.missShaderCount);
    RHI_RECORD_INPUT_POD_ARRAY(desc.missShaderRecordOverwrites, desc.missShaderCount);

    RHI_RECORD_INPUT_STRING_ARRAY(desc.hitGroupNames, desc.hitGroupCount);
    RHI_RECORD_INPUT_POD_ARRAY(desc.hitGroupRecordOverwrites, desc.hitGroupCount);

    RHI_RECORD_INPUT_STRING_ARRAY(desc.callableShaderEntryPointNames, desc.callableShaderCount);
    RHI_RECORD_INPUT_POD_ARRAY(desc.callableShaderRecordOverwrites, desc.callableShaderCount);

    ShaderTableDesc innerDesc = desc;
    innerDesc.program = getInnerObj(desc.program);
    RefPtr<RecordShaderTable> wrapped = new RecordShaderTable();
    auto result = baseObject->createShaderTable(innerDesc, wrapped->baseObject.writeRef());
    wrapOutput(wrapped, outTable);
    RHI_RECORD_OBJECT_OUTPUT(outTable);
    RHI_RECORD_RETURN(result);
}

} // namespace rhi::record
