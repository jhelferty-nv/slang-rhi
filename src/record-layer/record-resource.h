#pragma once

#include "record-base.h"
#include "reference.h"

namespace rhi::record {

// Forward declaration
class RecordTextureView;

// =============================================================================
// RecordBuffer
// =============================================================================

class RecordBuffer : public RecordObject<IBuffer>
{
public:
    virtual SLANG_NO_THROW const BufferDesc& SLANG_MCALL getDesc() override
    {
        return baseObject->getDesc();
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL getNativeHandle(NativeHandle* outHandle) override
    {
        return baseObject->getNativeHandle(outHandle);
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL getSharedHandle(NativeHandle* outHandle) override
    {
        return baseObject->getSharedHandle(outHandle);
    }

    virtual SLANG_NO_THROW DeviceAddress SLANG_MCALL getDeviceAddress() override
    {
        return baseObject->getDeviceAddress();
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL getDescriptorHandle(
        DescriptorHandleAccess access,
        Format format,
        BufferRange range,
        DescriptorHandle* outHandle) override
    {
        return baseObject->getDescriptorHandle(access, format, range, outHandle);
    }
};

// =============================================================================
// RecordTextureView (forward declared, defined before RecordTexture uses it)
// =============================================================================

class RecordTextureView : public RecordObject<ITextureView>
{
public:
    virtual SLANG_NO_THROW const TextureViewDesc& SLANG_MCALL getDesc() override
    {
        return baseObject->getDesc();
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL getNativeHandle(NativeHandle* outHandle) override
    {
        return baseObject->getNativeHandle(outHandle);
    }

    virtual SLANG_NO_THROW ITexture* SLANG_MCALL getTexture() override
    {
        return baseObject->getTexture();
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL
    getDescriptorHandle(DescriptorHandleAccess access, DescriptorHandle* outHandle) override
    {
        return baseObject->getDescriptorHandle(access, outHandle);
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL
    getCombinedTextureSamplerDescriptorHandle(DescriptorHandle* outHandle) override
    {
        return baseObject->getCombinedTextureSamplerDescriptorHandle(outHandle);
    }
};

// =============================================================================
// RecordTexture
// =============================================================================

class RecordTexture : public RecordObject<ITexture>
{
public:
    virtual SLANG_NO_THROW const TextureDesc& SLANG_MCALL getDesc() override
    {
        return baseObject->getDesc();
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL getNativeHandle(NativeHandle* outHandle) override
    {
        return baseObject->getNativeHandle(outHandle);
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL getSharedHandle(NativeHandle* outHandle) override
    {
        return baseObject->getSharedHandle(outHandle);
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL
    createView(const TextureViewDesc& desc, ITextureView** outTextureView) override
    {
        RHI_RECORD_CALL("ITexture::createView");
        RHI_RECORD_INPUT_DESC(desc);
        RHI_PREPARE_OUTPUT(outTextureView);
        RefPtr<RecordTextureView> wrapped = new RecordTextureView();
        auto result = baseObject->createView(desc, wrapped->baseObject.writeRef());
        if (wrapped->baseObject)
        {
            wrapped->registerSelf();
            returnComPtr(outTextureView, wrapped);
        }
        RHI_RECORD_OBJECT_OUTPUT(outTextureView);
        RHI_RECORD_RETURN(result);
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL getDefaultView(ITextureView** outTextureView) override
    {
        RHI_RECORD_CALL("ITexture::getDefaultView");
        RHI_PREPARE_OUTPUT(outTextureView);
        RefPtr<RecordTextureView> wrapped = new RecordTextureView();
        auto result = baseObject->getDefaultView(wrapped->baseObject.writeRef());
        if (wrapped->baseObject)
        {
            wrapped->registerSelf();
            returnComPtr(outTextureView, wrapped);
        }
        RHI_RECORD_OBJECT_OUTPUT(outTextureView);
        RHI_RECORD_RETURN(result);
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL getSubresourceLayout(
        uint32_t mip,
        size_t rowAlignment,
        SubresourceLayout* outLayout) override
    {
        return baseObject->getSubresourceLayout(mip, rowAlignment, outLayout);
    }
};

// =============================================================================
// RecordSampler
// =============================================================================

class RecordSampler : public RecordObject<ISampler>
{
public:
    virtual SLANG_NO_THROW const SamplerDesc& SLANG_MCALL getDesc() override
    {
        return baseObject->getDesc();
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL getNativeHandle(NativeHandle* outHandle) override
    {
        return baseObject->getNativeHandle(outHandle);
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL
    getDescriptorHandle(DescriptorHandle* outHandle) override
    {
        return baseObject->getDescriptorHandle(outHandle);
    }
};

// =============================================================================
// RecordAccelerationStructure
// =============================================================================

class RecordAccelerationStructure : public RecordObject<IAccelerationStructure>
{
public:
    virtual SLANG_NO_THROW Result SLANG_MCALL getNativeHandle(NativeHandle* outHandle) override
    {
        return baseObject->getNativeHandle(outHandle);
    }

    virtual SLANG_NO_THROW AccelerationStructureHandle SLANG_MCALL getHandle() override
    {
        return baseObject->getHandle();
    }

    virtual SLANG_NO_THROW DeviceAddress SLANG_MCALL getDeviceAddress() override
    {
        return baseObject->getDeviceAddress();
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL
    getDescriptorHandle(DescriptorHandle* outHandle) override
    {
        return baseObject->getDescriptorHandle(outHandle);
    }
};

} // namespace rhi::record
