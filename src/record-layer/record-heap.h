#pragma once

#include "record-base.h"

namespace rhi::record {

class RecordHeap : public RecordObject<IHeap>
{
public:
    virtual SLANG_NO_THROW Result SLANG_MCALL
    allocate(const HeapAllocDesc& desc, HeapAlloc* outAllocation) override
    {
        return baseObject->allocate(desc, outAllocation);
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL free(HeapAlloc allocation) override
    {
        return baseObject->free(allocation);
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL report(HeapReport* outReport) override
    {
        return baseObject->report(outReport);
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL flush() override
    {
        return baseObject->flush();
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL removeEmptyPages() override
    {
        return baseObject->removeEmptyPages();
    }
};

} // namespace rhi::record
