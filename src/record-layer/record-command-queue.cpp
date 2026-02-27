#include "record-command-queue.h"
#include "record-command-buffer.h"
#include "record-command-encoder.h"
#include "record-fence.h"

#include "reference.h"
#include "core/short_vector.h"

namespace rhi::record {

QueueType RecordCommandQueue::getType()
{
    return baseObject->getType();
}

Result RecordCommandQueue::createCommandEncoder(ICommandEncoder** outEncoder)
{
    RHI_RECORD_CALL("ICommandQueue::createCommandEncoder");
    RHI_PREPARE_OUTPUT(outEncoder);

    RefPtr<RecordCommandEncoder> wrapped = new RecordCommandEncoder();
    auto result = baseObject->createCommandEncoder(wrapped->baseObject.writeRef());
    if (wrapped->baseObject)
    {
        wrapped->registerSelf();
        returnComPtr(outEncoder, wrapped);
    }
    RHI_RECORD_OBJECT_OUTPUT(outEncoder);
    RHI_RECORD_RETURN(result);
}

Result RecordCommandQueue::submit(const SubmitDesc& desc)
{
    RHI_RECORD_CALL("ICommandQueue::submit");

    // Unwrap command buffers and fences for the real call
    SubmitDesc innerDesc = desc;
    short_vector<ICommandBuffer*> innerCmdBuffers;
    for (uint32_t i = 0; i < desc.commandBufferCount; i++)
        innerCmdBuffers.push_back(getInnerObj(desc.commandBuffers[i]));
    innerDesc.commandBuffers = innerCmdBuffers.data();

    short_vector<IFence*> innerWaitFences;
    for (uint32_t i = 0; i < desc.waitFenceCount; i++)
        innerWaitFences.push_back(getInnerObj(desc.waitFences[i]));
    innerDesc.waitFences = innerWaitFences.data();

    short_vector<IFence*> innerSignalFences;
    for (uint32_t i = 0; i < desc.signalFenceCount; i++)
        innerSignalFences.push_back(getInnerObj(desc.signalFences[i]));
    innerDesc.signalFences = innerSignalFences.data();

    RHI_RECORD_INPUT_UINT32(desc.commandBufferCount);
    for (uint32_t i = 0; i < desc.commandBufferCount; i++)
        RHI_RECORD_OBJECT_INPUT(desc.commandBuffers[i]);
    RHI_RECORD_INPUT_UINT32(desc.waitFenceCount);
    for (uint32_t i = 0; i < desc.waitFenceCount; i++)
    {
        RHI_RECORD_OBJECT_INPUT(desc.waitFences[i]);
        if (desc.waitFenceValues)
            RHI_RECORD_INPUT_POD(desc.waitFenceValues[i]);
    }
    RHI_RECORD_INPUT_UINT32(desc.signalFenceCount);
    for (uint32_t i = 0; i < desc.signalFenceCount; i++)
    {
        RHI_RECORD_OBJECT_INPUT(desc.signalFences[i]);
        if (desc.signalFenceValues)
            RHI_RECORD_INPUT_POD(desc.signalFenceValues[i]);
    }
    auto result = baseObject->submit(innerDesc);
    RHI_RECORD_RETURN(result);
}

Result RecordCommandQueue::waitOnHost()
{
    RHI_RECORD_CALL("ICommandQueue::waitOnHost");
    auto result = baseObject->waitOnHost();
    RHI_RECORD_RETURN(result);
}

Result RecordCommandQueue::getNativeHandle(NativeHandle* outHandle)
{
    return baseObject->getNativeHandle(outHandle);
}

} // namespace rhi::record
