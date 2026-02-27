#include "record-command-encoder.h"
#include "record-command-buffer.h"
#include "record-resource.h"
#include "record-pipeline.h"
#include "record-query.h"
#include "record-fence.h"
#include "record-shader-object.h"

#include "reference.h"
#include "core/short_vector.h"

namespace rhi::record {

// =============================================================================
// RecordCommandEncoder
// =============================================================================

IRenderPassEncoder* RecordCommandEncoder::beginRenderPass(const RenderPassDesc& desc)
{
    RHI_RECORD_CALL("ICommandEncoder::beginRenderPass");
    RHI_RECORD_INPUT_UINT32(desc.colorAttachmentCount);
    for (uint32_t i = 0; i < desc.colorAttachmentCount; i++)
    {
        RHI_RECORD_OBJECT_INPUT(desc.colorAttachments[i].view);
        RHI_RECORD_OBJECT_INPUT(desc.colorAttachments[i].resolveTarget);
        RHI_RECORD_INPUT_POD(desc.colorAttachments[i].loadOp);
        RHI_RECORD_INPUT_POD(desc.colorAttachments[i].storeOp);
        RHI_RECORD_INPUT_POD(desc.colorAttachments[i].clearValue);
    }
    bool hasDepth = desc.depthStencilAttachment != nullptr;
    RHI_RECORD_INPUT_BOOL(hasDepth);
    if (hasDepth)
    {
        RHI_RECORD_OBJECT_INPUT(desc.depthStencilAttachment->view);
        RHI_RECORD_INPUT_POD(desc.depthStencilAttachment->depthLoadOp);
        RHI_RECORD_INPUT_POD(desc.depthStencilAttachment->depthStoreOp);
        RHI_RECORD_INPUT_POD(desc.depthStencilAttachment->depthClearValue);
        RHI_RECORD_INPUT_BOOL(desc.depthStencilAttachment->depthReadOnly);
        RHI_RECORD_INPUT_POD(desc.depthStencilAttachment->stencilLoadOp);
        RHI_RECORD_INPUT_POD(desc.depthStencilAttachment->stencilStoreOp);
        RHI_RECORD_INPUT_POD(desc.depthStencilAttachment->stencilClearValue);
        RHI_RECORD_INPUT_BOOL(desc.depthStencilAttachment->stencilReadOnly);
    }

    // Unwrap texture views in the render pass desc
    RenderPassDesc innerDesc = desc;
    short_vector<RenderPassColorAttachment> innerColorAttachments;
    for (uint32_t i = 0; i < desc.colorAttachmentCount; i++)
    {
        RenderPassColorAttachment att = desc.colorAttachments[i];
        att.view = getInnerObj(att.view);
        att.resolveTarget = getInnerObj(att.resolveTarget);
        innerColorAttachments.push_back(att);
    }
    innerDesc.colorAttachments = innerColorAttachments.data();

    RenderPassDepthStencilAttachment innerDepthStencil;
    if (desc.depthStencilAttachment)
    {
        innerDepthStencil = *desc.depthStencilAttachment;
        innerDepthStencil.view = getInnerObj(innerDepthStencil.view);
        innerDesc.depthStencilAttachment = &innerDepthStencil;
    }

    auto* inner = baseObject->beginRenderPass(innerDesc);
    m_renderPassEncoder.baseObject = inner;
    return &m_renderPassEncoder;
}

IComputePassEncoder* RecordCommandEncoder::beginComputePass()
{
    RHI_RECORD_CALL("ICommandEncoder::beginComputePass");
    auto* inner = baseObject->beginComputePass();
    m_computePassEncoder.baseObject = inner;
    return &m_computePassEncoder;
}

IRayTracingPassEncoder* RecordCommandEncoder::beginRayTracingPass()
{
    RHI_RECORD_CALL("ICommandEncoder::beginRayTracingPass");
    auto* inner = baseObject->beginRayTracingPass();
    m_rayTracingPassEncoder.baseObject = inner;
    return &m_rayTracingPassEncoder;
}

void RecordCommandEncoder::copyBuffer(IBuffer* dst, Offset dstOffset, IBuffer* src, Offset srcOffset, Size size)
{
    RHI_RECORD_CALL("ICommandEncoder::copyBuffer");
    RHI_RECORD_OBJECT_INPUT(dst);
    RHI_RECORD_OBJECT_INPUT(src);
    RHI_RECORD_INPUT_POD(dstOffset);
    RHI_RECORD_INPUT_POD(srcOffset);
    RHI_RECORD_INPUT_POD(size);
    baseObject->copyBuffer(getInnerObj(dst), dstOffset, getInnerObj(src), srcOffset, size);
}

void RecordCommandEncoder::copyTexture(ITexture* dst, SubresourceRange dstSubresource, Offset3D dstOffset, ITexture* src, SubresourceRange srcSubresource, Offset3D srcOffset, Extent3D extent)
{
    RHI_RECORD_CALL("ICommandEncoder::copyTexture");
    RHI_RECORD_OBJECT_INPUT(dst);
    RHI_RECORD_OBJECT_INPUT(src);
    RHI_RECORD_INPUT_POD(dstSubresource);
    RHI_RECORD_INPUT_POD(dstOffset);
    RHI_RECORD_INPUT_POD(srcSubresource);
    RHI_RECORD_INPUT_POD(srcOffset);
    RHI_RECORD_INPUT_POD(extent);
    baseObject->copyTexture(getInnerObj(dst), dstSubresource, dstOffset, getInnerObj(src), srcSubresource, srcOffset, extent);
}

void RecordCommandEncoder::copyTextureToBuffer(IBuffer* dst, Offset dstOffset, Size dstSize, Size dstRowPitch, ITexture* src, uint32_t srcLayer, uint32_t srcMip, Offset3D srcOffset, Extent3D extent)
{
    RHI_RECORD_CALL("ICommandEncoder::copyTextureToBuffer");
    RHI_RECORD_OBJECT_INPUT(dst);
    RHI_RECORD_OBJECT_INPUT(src);
    baseObject->copyTextureToBuffer(getInnerObj(dst), dstOffset, dstSize, dstRowPitch, getInnerObj(src), srcLayer, srcMip, srcOffset, extent);
}

void RecordCommandEncoder::copyBufferToTexture(ITexture* dst, uint32_t dstLayer, uint32_t dstMip, Offset3D dstOffset, IBuffer* src, Offset srcOffset, Size srcSize, Size srcRowPitch, Extent3D extent)
{
    RHI_RECORD_CALL("ICommandEncoder::copyBufferToTexture");
    RHI_RECORD_OBJECT_INPUT(dst);
    RHI_RECORD_OBJECT_INPUT(src);
    baseObject->copyBufferToTexture(getInnerObj(dst), dstLayer, dstMip, dstOffset, getInnerObj(src), srcOffset, srcSize, srcRowPitch, extent);
}

Result RecordCommandEncoder::uploadTextureData(ITexture* dst, SubresourceRange subresourceRange, Offset3D offset, Extent3D extent, const SubresourceData* subresourceData, uint32_t subresourceDataCount)
{
    RHI_RECORD_CALL("ICommandEncoder::uploadTextureData");
    RHI_RECORD_OBJECT_INPUT(dst);
    RHI_RECORD_INPUT_POD(subresourceRange);
    RHI_RECORD_INPUT_POD(offset);
    RHI_RECORD_INPUT_POD(extent);
    RHI_RECORD_INPUT_UINT32(subresourceDataCount);
    for (uint32_t i = 0; i < subresourceDataCount; i++)
    {
        size_t dataSize = subresourceData[i].slicePitch * extent.depth;
        RHI_RECORD_INPUT_POD(subresourceData[i].rowPitch);
        RHI_RECORD_INPUT_POD(subresourceData[i].slicePitch);
        RHI_RECORD_INPUT_BLOB(subresourceData[i].data, dataSize);
    }
    return baseObject->uploadTextureData(getInnerObj(dst), subresourceRange, offset, extent, subresourceData, subresourceDataCount);
}

Result RecordCommandEncoder::uploadBufferData(IBuffer* dst, Offset offset, Size size, const void* data)
{
    RHI_RECORD_CALL("ICommandEncoder::uploadBufferData");
    RHI_RECORD_OBJECT_INPUT(dst);
    RHI_RECORD_INPUT_POD(offset);
    RHI_RECORD_INPUT_POD(size);
    RHI_RECORD_INPUT_BLOB(data, size);
    return baseObject->uploadBufferData(getInnerObj(dst), offset, size, data);
}

void RecordCommandEncoder::clearBuffer(IBuffer* buffer, BufferRange range)
{
    RHI_RECORD_CALL("ICommandEncoder::clearBuffer");
    RHI_RECORD_OBJECT_INPUT(buffer);
    RHI_RECORD_INPUT_POD(range);
    baseObject->clearBuffer(getInnerObj(buffer), range);
}

void RecordCommandEncoder::clearTextureFloat(ITexture* texture, SubresourceRange subresourceRange, float clearValue[4])
{
    RHI_RECORD_CALL("ICommandEncoder::clearTextureFloat");
    RHI_RECORD_OBJECT_INPUT(texture);
    baseObject->clearTextureFloat(getInnerObj(texture), subresourceRange, clearValue);
}

void RecordCommandEncoder::clearTextureUint(ITexture* texture, SubresourceRange subresourceRange, uint32_t clearValue[4])
{
    RHI_RECORD_CALL("ICommandEncoder::clearTextureUint");
    RHI_RECORD_OBJECT_INPUT(texture);
    baseObject->clearTextureUint(getInnerObj(texture), subresourceRange, clearValue);
}

void RecordCommandEncoder::clearTextureSint(ITexture* texture, SubresourceRange subresourceRange, int32_t clearValue[4])
{
    RHI_RECORD_CALL("ICommandEncoder::clearTextureSint");
    RHI_RECORD_OBJECT_INPUT(texture);
    baseObject->clearTextureSint(getInnerObj(texture), subresourceRange, clearValue);
}

void RecordCommandEncoder::clearTextureDepthStencil(ITexture* texture, SubresourceRange subresourceRange, bool clearDepth, float depthValue, bool clearStencil, uint8_t stencilValue)
{
    RHI_RECORD_CALL("ICommandEncoder::clearTextureDepthStencil");
    RHI_RECORD_OBJECT_INPUT(texture);
    baseObject->clearTextureDepthStencil(getInnerObj(texture), subresourceRange, clearDepth, depthValue, clearStencil, stencilValue);
}

void RecordCommandEncoder::resolveQuery(IQueryPool* queryPool, uint32_t index, uint32_t count, IBuffer* buffer, uint64_t offset)
{
    RHI_RECORD_CALL("ICommandEncoder::resolveQuery");
    RHI_RECORD_OBJECT_INPUT(queryPool);
    RHI_RECORD_OBJECT_INPUT(buffer);
    baseObject->resolveQuery(getInnerObj(queryPool), index, count, getInnerObj(buffer), offset);
}

static AccelerationStructureBuildInput unwrapBuildInput(const AccelerationStructureBuildInput& input)
{
    AccelerationStructureBuildInput out = input;
    switch (input.type)
    {
    case AccelerationStructureBuildInputType::Instances:
        out.instances.instanceBuffer = getInnerBufferOffsetPair(input.instances.instanceBuffer);
        break;
    case AccelerationStructureBuildInputType::Triangles:
        for (uint32_t i = 0; i < input.triangles.vertexBufferCount; i++)
            out.triangles.vertexBuffers[i] = getInnerBufferOffsetPair(input.triangles.vertexBuffers[i]);
        out.triangles.indexBuffer = getInnerBufferOffsetPair(input.triangles.indexBuffer);
        out.triangles.preTransformBuffer = getInnerBufferOffsetPair(input.triangles.preTransformBuffer);
        break;
    case AccelerationStructureBuildInputType::ProceduralPrimitives:
        for (uint32_t i = 0; i < input.proceduralPrimitives.aabbBufferCount; i++)
            out.proceduralPrimitives.aabbBuffers[i] = getInnerBufferOffsetPair(input.proceduralPrimitives.aabbBuffers[i]);
        break;
    case AccelerationStructureBuildInputType::Spheres:
        for (uint32_t i = 0; i < input.spheres.vertexBufferCount; i++)
        {
            out.spheres.vertexPositionBuffers[i] = getInnerBufferOffsetPair(input.spheres.vertexPositionBuffers[i]);
            out.spheres.vertexRadiusBuffers[i] = getInnerBufferOffsetPair(input.spheres.vertexRadiusBuffers[i]);
        }
        out.spheres.indexBuffer = getInnerBufferOffsetPair(input.spheres.indexBuffer);
        break;
    case AccelerationStructureBuildInputType::LinearSweptSpheres:
        for (uint32_t i = 0; i < input.linearSweptSpheres.vertexBufferCount; i++)
        {
            out.linearSweptSpheres.vertexPositionBuffers[i] = getInnerBufferOffsetPair(input.linearSweptSpheres.vertexPositionBuffers[i]);
            out.linearSweptSpheres.vertexRadiusBuffers[i] = getInnerBufferOffsetPair(input.linearSweptSpheres.vertexRadiusBuffers[i]);
        }
        out.linearSweptSpheres.indexBuffer = getInnerBufferOffsetPair(input.linearSweptSpheres.indexBuffer);
        break;
    }
    return out;
}

void RecordCommandEncoder::buildAccelerationStructure(const AccelerationStructureBuildDesc& desc, IAccelerationStructure* dst, IAccelerationStructure* src, BufferOffsetPair scratchBuffer, uint32_t propertyQueryCount, const AccelerationStructureQueryDesc* queryDescs)
{
    RHI_RECORD_CALL("ICommandEncoder::buildAccelerationStructure");
    RHI_RECORD_OBJECT_INPUT(dst);
    RHI_RECORD_OBJECT_INPUT(src);

    AccelerationStructureBuildDesc innerDesc = desc;
    short_vector<AccelerationStructureBuildInput> innerInputs;
    for (uint32_t i = 0; i < desc.inputCount; i++)
        innerInputs.push_back(unwrapBuildInput(desc.inputs[i]));
    innerDesc.inputs = innerInputs.data();

    short_vector<AccelerationStructureQueryDesc> innerQueryDescs;
    for (uint32_t i = 0; i < propertyQueryCount; i++)
    {
        AccelerationStructureQueryDesc d = queryDescs[i];
        d.queryPool = getInnerObj(d.queryPool);
        innerQueryDescs.push_back(d);
    }

    baseObject->buildAccelerationStructure(
        innerDesc,
        getInnerObj(dst),
        getInnerObj(src),
        getInnerBufferOffsetPair(scratchBuffer),
        propertyQueryCount,
        propertyQueryCount > 0 ? innerQueryDescs.data() : nullptr);
}

void RecordCommandEncoder::copyAccelerationStructure(IAccelerationStructure* dst, IAccelerationStructure* src, AccelerationStructureCopyMode mode)
{
    RHI_RECORD_CALL("ICommandEncoder::copyAccelerationStructure");
    RHI_RECORD_OBJECT_INPUT(dst);
    RHI_RECORD_OBJECT_INPUT(src);
    baseObject->copyAccelerationStructure(getInnerObj(dst), getInnerObj(src), mode);
}

void RecordCommandEncoder::queryAccelerationStructureProperties(uint32_t accelerationStructureCount, IAccelerationStructure** accelerationStructures, uint32_t queryCount, const AccelerationStructureQueryDesc* queryDescs)
{
    RHI_RECORD_CALL("ICommandEncoder::queryAccelerationStructureProperties");
    short_vector<IAccelerationStructure*> innerAS;
    for (uint32_t i = 0; i < accelerationStructureCount; i++)
        innerAS.push_back(getInnerObj(accelerationStructures[i]));

    short_vector<AccelerationStructureQueryDesc> innerQueryDescs;
    for (uint32_t i = 0; i < queryCount; i++)
    {
        AccelerationStructureQueryDesc d = queryDescs[i];
        d.queryPool = getInnerObj(d.queryPool);
        innerQueryDescs.push_back(d);
    }

    baseObject->queryAccelerationStructureProperties(
        accelerationStructureCount,
        innerAS.data(),
        queryCount,
        queryCount > 0 ? innerQueryDescs.data() : nullptr);
}

void RecordCommandEncoder::serializeAccelerationStructure(BufferOffsetPair dst, IAccelerationStructure* src)
{
    RHI_RECORD_CALL("ICommandEncoder::serializeAccelerationStructure");
    RHI_RECORD_OBJECT_INPUT(src);
    baseObject->serializeAccelerationStructure(getInnerBufferOffsetPair(dst), getInnerObj(src));
}

void RecordCommandEncoder::deserializeAccelerationStructure(IAccelerationStructure* dst, BufferOffsetPair src)
{
    RHI_RECORD_CALL("ICommandEncoder::deserializeAccelerationStructure");
    RHI_RECORD_OBJECT_INPUT(dst);
    baseObject->deserializeAccelerationStructure(getInnerObj(dst), getInnerBufferOffsetPair(src));
}

void RecordCommandEncoder::executeClusterOperation(const ClusterOperationDesc& desc)
{
    RHI_RECORD_CALL("ICommandEncoder::executeClusterOperation");
    RHI_RECORD_INPUT_POD(desc.params);
    RHI_RECORD_OBJECT_INPUT(desc.argCountBuffer.buffer);
    RHI_RECORD_INPUT_POD(desc.argCountBuffer.offset);
    RHI_RECORD_OBJECT_INPUT(desc.argsBuffer.buffer);
    RHI_RECORD_INPUT_POD(desc.argsBuffer.offset);
    RHI_RECORD_OBJECT_INPUT(desc.scratchBuffer.buffer);
    RHI_RECORD_INPUT_POD(desc.scratchBuffer.offset);
    RHI_RECORD_OBJECT_INPUT(desc.addressesBuffer.buffer);
    RHI_RECORD_INPUT_POD(desc.addressesBuffer.offset);
    RHI_RECORD_OBJECT_INPUT(desc.resultBuffer.buffer);
    RHI_RECORD_INPUT_POD(desc.resultBuffer.offset);
    RHI_RECORD_OBJECT_INPUT(desc.sizesBuffer.buffer);
    RHI_RECORD_INPUT_POD(desc.sizesBuffer.offset);

    ClusterOperationDesc innerDesc = desc;
    innerDesc.argCountBuffer = getInnerBufferOffsetPair(desc.argCountBuffer);
    innerDesc.argsBuffer = getInnerBufferOffsetPair(desc.argsBuffer);
    innerDesc.scratchBuffer = getInnerBufferOffsetPair(desc.scratchBuffer);
    innerDesc.addressesBuffer = getInnerBufferOffsetPair(desc.addressesBuffer);
    innerDesc.resultBuffer = getInnerBufferOffsetPair(desc.resultBuffer);
    innerDesc.sizesBuffer = getInnerBufferOffsetPair(desc.sizesBuffer);
    baseObject->executeClusterOperation(innerDesc);
}

void RecordCommandEncoder::convertCooperativeVectorMatrix(IBuffer* dstBuffer, const CooperativeVectorMatrixDesc* dstDescs, IBuffer* srcBuffer, const CooperativeVectorMatrixDesc* srcDescs, uint32_t matrixCount)
{
    RHI_RECORD_CALL("ICommandEncoder::convertCooperativeVectorMatrix");
    RHI_RECORD_OBJECT_INPUT(dstBuffer);
    RHI_RECORD_OBJECT_INPUT(srcBuffer);
    RHI_RECORD_INPUT_UINT32(matrixCount);
    for (uint32_t i = 0; i < matrixCount; i++)
    {
        RHI_RECORD_INPUT_POD(dstDescs[i]);
        RHI_RECORD_INPUT_POD(srcDescs[i]);
    }
    baseObject->convertCooperativeVectorMatrix(getInnerObj(dstBuffer), dstDescs, getInnerObj(srcBuffer), srcDescs, matrixCount);
}

void RecordCommandEncoder::setBufferState(IBuffer* buffer, ResourceState state)
{
    RHI_RECORD_CALL("ICommandEncoder::setBufferState");
    RHI_RECORD_OBJECT_INPUT(buffer);
    RHI_RECORD_INPUT_POD(state);
    baseObject->setBufferState(getInnerObj(buffer), state);
}

void RecordCommandEncoder::setTextureState(ITexture* texture, SubresourceRange subresourceRange, ResourceState state)
{
    RHI_RECORD_CALL("ICommandEncoder::setTextureState");
    RHI_RECORD_OBJECT_INPUT(texture);
    RHI_RECORD_INPUT_POD(subresourceRange);
    RHI_RECORD_INPUT_POD(state);
    baseObject->setTextureState(getInnerObj(texture), subresourceRange, state);
}

void RecordCommandEncoder::globalBarrier()
{
    RHI_RECORD_CALL("ICommandEncoder::globalBarrier");
    baseObject->globalBarrier();
}

void RecordCommandEncoder::pushDebugGroup(const char* name, const MarkerColor& color)
{
    baseObject->pushDebugGroup(name, color);
}

void RecordCommandEncoder::popDebugGroup()
{
    baseObject->popDebugGroup();
}

void RecordCommandEncoder::insertDebugMarker(const char* name, const MarkerColor& color)
{
    baseObject->insertDebugMarker(name, color);
}

void RecordCommandEncoder::writeTimestamp(IQueryPool* queryPool, uint32_t queryIndex)
{
    RHI_RECORD_CALL("ICommandEncoder::writeTimestamp");
    RHI_RECORD_OBJECT_INPUT(queryPool);
    RHI_RECORD_INPUT_UINT32(queryIndex);
    baseObject->writeTimestamp(getInnerObj(queryPool), queryIndex);
}

Result RecordCommandEncoder::finish(ICommandBuffer** outCommandBuffer)
{
    RHI_RECORD_CALL("ICommandEncoder::finish");
    RHI_PREPARE_OUTPUT(outCommandBuffer);
    RefPtr<RecordCommandBuffer> wrapped = new RecordCommandBuffer();
    auto result = baseObject->finish(wrapped->baseObject.writeRef());
    if (wrapped->baseObject)
    {
        wrapped->registerSelf();
        returnComPtr(outCommandBuffer, wrapped);
    }
    RHI_RECORD_OBJECT_OUTPUT(outCommandBuffer);
    RHI_RECORD_RETURN(result);
}

Result RecordCommandEncoder::getNativeHandle(NativeHandle* outHandle)
{
    return baseObject->getNativeHandle(outHandle);
}

// =============================================================================
// RecordRenderPassEncoder
// =============================================================================

IShaderObject* RecordRenderPassEncoder::bindPipeline(IRenderPipeline* pipeline)
{
    RHI_RECORD_CALL("IRenderPassEncoder::bindPipeline");
    RHI_RECORD_OBJECT_INPUT(pipeline);
    m_rootObject->reset();
    m_rootObject->baseObject = baseObject->bindPipeline(getInnerObj(pipeline));
    m_rootObject->registerSelf();
    slangRecord_recordHandle(SLANG_RECORD_FLAG_OUTPUT, static_cast<ISlangUnknown*>(static_cast<IShaderObject*>(m_rootObject.get())));
    return m_rootObject;
}

void RecordRenderPassEncoder::bindPipeline(IRenderPipeline* pipeline, IShaderObject* rootObject)
{
    RHI_RECORD_CALL("IRenderPassEncoder::bindPipelineWithObject");
    RHI_RECORD_OBJECT_INPUT(pipeline);
    RHI_RECORD_OBJECT_INPUT(rootObject);
    baseObject->bindPipeline(getInnerObj(pipeline), getInnerObj(rootObject));
}

void RecordRenderPassEncoder::setRenderState(const RenderState& state)
{
    RHI_RECORD_CALL("IRenderPassEncoder::setRenderState");
    RHI_RECORD_INPUT_POD(state);

    RenderState innerState = state;
    for (uint32_t i = 0; i < state.vertexBufferCount; i++)
        innerState.vertexBuffers[i] = getInnerBufferOffsetPair(state.vertexBuffers[i]);
    innerState.indexBuffer = getInnerBufferOffsetPair(state.indexBuffer);
    baseObject->setRenderState(innerState);
}

void RecordRenderPassEncoder::draw(const DrawArguments& args)
{
    RHI_RECORD_CALL("IRenderPassEncoder::draw");
    RHI_RECORD_INPUT_POD(args);
    baseObject->draw(args);
}

void RecordRenderPassEncoder::drawIndexed(const DrawArguments& args)
{
    RHI_RECORD_CALL("IRenderPassEncoder::drawIndexed");
    RHI_RECORD_INPUT_POD(args);
    baseObject->drawIndexed(args);
}

void RecordRenderPassEncoder::drawIndirect(uint32_t maxDrawCount, BufferOffsetPair argBuffer, BufferOffsetPair countBuffer)
{
    RHI_RECORD_CALL("IRenderPassEncoder::drawIndirect");
    RHI_RECORD_INPUT_UINT32(maxDrawCount);
    RHI_RECORD_INPUT_POD(argBuffer);
    RHI_RECORD_INPUT_POD(countBuffer);
    baseObject->drawIndirect(maxDrawCount, getInnerBufferOffsetPair(argBuffer), getInnerBufferOffsetPair(countBuffer));
}

void RecordRenderPassEncoder::drawIndexedIndirect(uint32_t maxDrawCount, BufferOffsetPair argBuffer, BufferOffsetPair countBuffer)
{
    RHI_RECORD_CALL("IRenderPassEncoder::drawIndexedIndirect");
    RHI_RECORD_INPUT_UINT32(maxDrawCount);
    RHI_RECORD_INPUT_POD(argBuffer);
    RHI_RECORD_INPUT_POD(countBuffer);
    baseObject->drawIndexedIndirect(maxDrawCount, getInnerBufferOffsetPair(argBuffer), getInnerBufferOffsetPair(countBuffer));
}

void RecordRenderPassEncoder::drawMeshTasks(uint32_t x, uint32_t y, uint32_t z)
{
    RHI_RECORD_CALL("IRenderPassEncoder::drawMeshTasks");
    RHI_RECORD_INPUT_UINT32(x);
    RHI_RECORD_INPUT_UINT32(y);
    RHI_RECORD_INPUT_UINT32(z);
    baseObject->drawMeshTasks(x, y, z);
}

void RecordRenderPassEncoder::pushDebugGroup(const char* name, const MarkerColor& color) { baseObject->pushDebugGroup(name, color); }
void RecordRenderPassEncoder::popDebugGroup() { baseObject->popDebugGroup(); }
void RecordRenderPassEncoder::insertDebugMarker(const char* name, const MarkerColor& color) { baseObject->insertDebugMarker(name, color); }
void RecordRenderPassEncoder::writeTimestamp(IQueryPool* queryPool, uint32_t queryIndex) { baseObject->writeTimestamp(getInnerObj(queryPool), queryIndex); }
void RecordRenderPassEncoder::end() { baseObject->end(); }

// =============================================================================
// RecordComputePassEncoder
// =============================================================================

IShaderObject* RecordComputePassEncoder::bindPipeline(IComputePipeline* pipeline)
{
    RHI_RECORD_CALL("IComputePassEncoder::bindPipeline");
    RHI_RECORD_OBJECT_INPUT(pipeline);
    m_rootObject->reset();
    m_rootObject->baseObject = baseObject->bindPipeline(getInnerObj(pipeline));
    m_rootObject->registerSelf();
    slangRecord_recordHandle(SLANG_RECORD_FLAG_OUTPUT, static_cast<ISlangUnknown*>(static_cast<IShaderObject*>(m_rootObject.get())));
    return m_rootObject;
}

void RecordComputePassEncoder::bindPipeline(IComputePipeline* pipeline, IShaderObject* rootObject)
{
    RHI_RECORD_CALL("IComputePassEncoder::bindPipelineWithObject");
    RHI_RECORD_OBJECT_INPUT(pipeline);
    RHI_RECORD_OBJECT_INPUT(rootObject);
    baseObject->bindPipeline(getInnerObj(pipeline), getInnerObj(rootObject));
}

void RecordComputePassEncoder::dispatchCompute(uint32_t x, uint32_t y, uint32_t z)
{
    RHI_RECORD_CALL("IComputePassEncoder::dispatchCompute");
    RHI_RECORD_INPUT_UINT32(x);
    RHI_RECORD_INPUT_UINT32(y);
    RHI_RECORD_INPUT_UINT32(z);
    baseObject->dispatchCompute(x, y, z);
}

void RecordComputePassEncoder::dispatchComputeIndirect(BufferOffsetPair argBuffer)
{
    RHI_RECORD_CALL("IComputePassEncoder::dispatchComputeIndirect");
    RHI_RECORD_INPUT_POD(argBuffer);
    baseObject->dispatchComputeIndirect(getInnerBufferOffsetPair(argBuffer));
}

void RecordComputePassEncoder::pushDebugGroup(const char* name, const MarkerColor& color) { baseObject->pushDebugGroup(name, color); }
void RecordComputePassEncoder::popDebugGroup() { baseObject->popDebugGroup(); }
void RecordComputePassEncoder::insertDebugMarker(const char* name, const MarkerColor& color) { baseObject->insertDebugMarker(name, color); }
void RecordComputePassEncoder::writeTimestamp(IQueryPool* queryPool, uint32_t queryIndex) { baseObject->writeTimestamp(getInnerObj(queryPool), queryIndex); }
void RecordComputePassEncoder::end() { baseObject->end(); }

// =============================================================================
// RecordRayTracingPassEncoder
// =============================================================================

IShaderObject* RecordRayTracingPassEncoder::bindPipeline(IRayTracingPipeline* pipeline, IShaderTable* shaderTable)
{
    RHI_RECORD_CALL("IRayTracingPassEncoder::bindPipeline");
    RHI_RECORD_OBJECT_INPUT(pipeline);
    RHI_RECORD_OBJECT_INPUT(shaderTable);
    m_rootObject->reset();
    m_rootObject->baseObject = baseObject->bindPipeline(getInnerObj(pipeline), getInnerObj(shaderTable));
    m_rootObject->registerSelf();
    slangRecord_recordHandle(SLANG_RECORD_FLAG_OUTPUT, static_cast<ISlangUnknown*>(static_cast<IShaderObject*>(m_rootObject.get())));
    return m_rootObject;
}

void RecordRayTracingPassEncoder::bindPipeline(IRayTracingPipeline* pipeline, IShaderTable* shaderTable, IShaderObject* rootObject)
{
    RHI_RECORD_CALL("IRayTracingPassEncoder::bindPipelineWithObject");
    RHI_RECORD_OBJECT_INPUT(pipeline);
    RHI_RECORD_OBJECT_INPUT(shaderTable);
    RHI_RECORD_OBJECT_INPUT(rootObject);
    baseObject->bindPipeline(getInnerObj(pipeline), getInnerObj(shaderTable), getInnerObj(rootObject));
}

void RecordRayTracingPassEncoder::dispatchRays(uint32_t rayGenShaderIndex, uint32_t width, uint32_t height, uint32_t depth)
{
    RHI_RECORD_CALL("IRayTracingPassEncoder::dispatchRays");
    RHI_RECORD_INPUT_UINT32(rayGenShaderIndex);
    RHI_RECORD_INPUT_UINT32(width);
    RHI_RECORD_INPUT_UINT32(height);
    RHI_RECORD_INPUT_UINT32(depth);
    baseObject->dispatchRays(rayGenShaderIndex, width, height, depth);
}

void RecordRayTracingPassEncoder::pushDebugGroup(const char* name, const MarkerColor& color) { baseObject->pushDebugGroup(name, color); }
void RecordRayTracingPassEncoder::popDebugGroup() { baseObject->popDebugGroup(); }
void RecordRayTracingPassEncoder::insertDebugMarker(const char* name, const MarkerColor& color) { baseObject->insertDebugMarker(name, color); }
void RecordRayTracingPassEncoder::writeTimestamp(IQueryPool* queryPool, uint32_t queryIndex) { baseObject->writeTimestamp(getInnerObj(queryPool), queryIndex); }
void RecordRayTracingPassEncoder::end() { baseObject->end(); }

} // namespace rhi::record
