#pragma once

#include "record-base.h"
#include "record-resource.h"

namespace rhi::record {

class RecordSurface : public RecordObject<ISurface>
{
public:
    virtual SLANG_NO_THROW const SurfaceInfo& SLANG_MCALL getInfo() override
    {
        return baseObject->getInfo();
    }

    virtual SLANG_NO_THROW const SurfaceConfig* SLANG_MCALL getConfig() override
    {
        return baseObject->getConfig();
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL configure(const SurfaceConfig& config) override
    {
        RHI_RECORD_CALL("ISurface::configure");
        RHI_RECORD_INPUT_POD(config);
        auto result = baseObject->configure(config);
        RHI_RECORD_RETURN(result);
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL unconfigure() override
    {
        RHI_RECORD_CALL("ISurface::unconfigure");
        auto result = baseObject->unconfigure();
        RHI_RECORD_RETURN(result);
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL acquireNextImage(ITexture** outTexture) override
    {
        RHI_RECORD_CALL("ISurface::acquireNextImage");
        RHI_PREPARE_OUTPUT(outTexture);
        RefPtr<RecordTexture> wrapped = new RecordTexture();
        auto result = baseObject->acquireNextImage(wrapped->baseObject.writeRef());
        if (wrapped->baseObject)
        {
            wrapped->registerSelf();
            returnComPtr(outTexture, wrapped);
        }
        RHI_RECORD_OBJECT_OUTPUT(outTexture);
        RHI_RECORD_RETURN(result);
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL present() override
    {
        RHI_RECORD_CALL("ISurface::present");
        auto result = baseObject->present();
        RHI_RECORD_RETURN(result);
    }
};

} // namespace rhi::record
