#pragma once

#include "record-base.h"

namespace rhi::record {

class RecordCommandBuffer : public RecordObject<ICommandBuffer>
{
public:
    virtual SLANG_NO_THROW Result SLANG_MCALL getNativeHandle(NativeHandle* outHandle) override
    {
        return baseObject->getNativeHandle(outHandle);
    }
};

} // namespace rhi::record
