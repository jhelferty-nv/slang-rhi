#pragma once

#include "record-base.h"

namespace rhi::record {

class RecordQueryPool : public RecordObject<IQueryPool>
{
public:
    virtual SLANG_NO_THROW const QueryPoolDesc& SLANG_MCALL getDesc() override
    {
        return baseObject->getDesc();
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL
    getResult(uint32_t queryIndex, uint32_t count, uint64_t* data) override
    {
        return baseObject->getResult(queryIndex, count, data);
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL reset() override
    {
        RHI_RECORD_CALL("IQueryPool::reset");
        auto result = baseObject->reset();
        RHI_RECORD_RETURN(result);
    }
};

} // namespace rhi::record
