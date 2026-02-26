#pragma once

#include "record-base.h"

namespace rhi::record {

class RecordFence : public RecordObject<IFence>
{
public:
    virtual SLANG_NO_THROW Result SLANG_MCALL getCurrentValue(uint64_t* outValue) override
    {
        RHI_RECORD_CALL("IFence::getCurrentValue");
        RHI_PREPARE_OUTPUT(outValue);
        auto result = baseObject->getCurrentValue(outValue);
        RHI_RECORD_OUTPUT(*outValue);
        RHI_RECORD_RETURN(result);
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL setCurrentValue(uint64_t value) override
    {
        RHI_RECORD_CALL("IFence::setCurrentValue");
        RHI_RECORD_INPUT_UINT64(value);
        auto result = baseObject->setCurrentValue(value);
        RHI_RECORD_RETURN(result);
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL getNativeHandle(NativeHandle* outHandle) override
    {
        return baseObject->getNativeHandle(outHandle);
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL getSharedHandle(NativeHandle* outHandle) override
    {
        return baseObject->getSharedHandle(outHandle);
    }
};

} // namespace rhi::record
