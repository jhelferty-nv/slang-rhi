#include "record-command-queue.h"
#include "record-command-buffer.h"
#include "record-command-encoder.h"
#include "record-fence.h"

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

    auto result = baseObject->createCommandEncoder(outEncoder);
    if (SLANG_SUCCEEDED(result) && *outEncoder)
    {
        auto* wrapped = new RecordCommandEncoder();
        wrapped->baseObject = *outEncoder;
        wrapped->registerSelf();
        *outEncoder = wrapped;
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

    RHI_RECORD_INPUT_POD(desc);
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
