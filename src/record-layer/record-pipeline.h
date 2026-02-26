#pragma once

#include "record-base.h"

namespace rhi::record {

// =============================================================================
// RecordShaderProgram
// =============================================================================

class RecordShaderProgram : public RecordObject<IShaderProgram>
{
public:
    virtual SLANG_NO_THROW const ShaderProgramDesc& SLANG_MCALL getDesc() override
    {
        return baseObject->getDesc();
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL
    getCompilationReport(ISlangBlob** outReportBlob) override
    {
        return baseObject->getCompilationReport(outReportBlob);
    }

    virtual SLANG_NO_THROW slang::TypeReflection* SLANG_MCALL
    findTypeByName(const char* name) override
    {
        return baseObject->findTypeByName(name);
    }
};

// =============================================================================
// RecordInputLayout
// =============================================================================

class RecordInputLayout : public RecordObject<IInputLayout>
{
public:
};

// =============================================================================
// RecordShaderTable
// =============================================================================

class RecordShaderTable : public RecordObject<IShaderTable>
{
public:
};

// =============================================================================
// RecordRenderPipeline
// =============================================================================

class RecordRenderPipeline : public RecordObject<IRenderPipeline>
{
public:
    virtual SLANG_NO_THROW const RenderPipelineDesc& SLANG_MCALL getDesc() override
    {
        return baseObject->getDesc();
    }

    virtual SLANG_NO_THROW IShaderProgram* SLANG_MCALL getProgram() override
    {
        return baseObject->getProgram();
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL getNativeHandle(NativeHandle* outHandle) override
    {
        return baseObject->getNativeHandle(outHandle);
    }
};

// =============================================================================
// RecordComputePipeline
// =============================================================================

class RecordComputePipeline : public RecordObject<IComputePipeline>
{
public:
    virtual SLANG_NO_THROW const ComputePipelineDesc& SLANG_MCALL getDesc() override
    {
        return baseObject->getDesc();
    }

    virtual SLANG_NO_THROW IShaderProgram* SLANG_MCALL getProgram() override
    {
        return baseObject->getProgram();
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL getNativeHandle(NativeHandle* outHandle) override
    {
        return baseObject->getNativeHandle(outHandle);
    }
};

// =============================================================================
// RecordRayTracingPipeline
// =============================================================================

class RecordRayTracingPipeline : public RecordObject<IRayTracingPipeline>
{
public:
    virtual SLANG_NO_THROW const RayTracingPipelineDesc& SLANG_MCALL getDesc() override
    {
        return baseObject->getDesc();
    }

    virtual SLANG_NO_THROW IShaderProgram* SLANG_MCALL getProgram() override
    {
        return baseObject->getProgram();
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL getNativeHandle(NativeHandle* outHandle) override
    {
        return baseObject->getNativeHandle(outHandle);
    }
};

} // namespace rhi::record
