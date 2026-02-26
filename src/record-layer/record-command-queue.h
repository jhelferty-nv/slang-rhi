#pragma once

#include "record-base.h"

namespace rhi::record {

class RecordCommandEncoder;

class RecordCommandQueue : public RecordObject<ICommandQueue>
{
public:
    virtual SLANG_NO_THROW QueueType SLANG_MCALL getType() override;
    virtual SLANG_NO_THROW Result SLANG_MCALL createCommandEncoder(ICommandEncoder** outEncoder) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL submit(const SubmitDesc& desc) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL waitOnHost() override;
    virtual SLANG_NO_THROW Result SLANG_MCALL getNativeHandle(NativeHandle* outHandle) override;
};

} // namespace rhi::record
