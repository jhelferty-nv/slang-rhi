#pragma once

#include "record-base.h"
#include "reference.h"

#include <unordered_map>
#include <vector>

namespace rhi::record {

class RecordShaderObject : public RecordObject<IShaderObject>
{
public:
    virtual SLANG_NO_THROW slang::TypeLayoutReflection* SLANG_MCALL getElementTypeLayout() override
    {
        return baseObject->getElementTypeLayout();
    }

    virtual SLANG_NO_THROW ShaderObjectContainerType SLANG_MCALL getContainerType() override
    {
        return baseObject->getContainerType();
    }

    virtual SLANG_NO_THROW uint32_t SLANG_MCALL getEntryPointCount() override
    {
        return baseObject->getEntryPointCount();
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL getEntryPoint(uint32_t index, IShaderObject** entryPoint) override
    {
        if (m_entryPoints.empty())
        {
            uint32_t count = getEntryPointCount();
            for (uint32_t i = 0; i < count; i++)
            {
                RefPtr<RecordShaderObject> wrapped = new RecordShaderObject();
                SLANG_RETURN_ON_FAIL(baseObject->getEntryPoint(i, wrapped->baseObject.writeRef()));
                wrapped->registerSelf();
                m_entryPoints.push_back(ComPtr<RecordShaderObject>(wrapped.get()));
            }
        }
        if (index >= m_entryPoints.size())
            return SLANG_FAIL;
        *entryPoint = m_entryPoints[index];
        m_entryPoints[index]->addRef();
        return SLANG_OK;
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL setData(const ShaderOffset& offset, const void* data, size_t size) override
    {
        RHI_RECORD_CALL("IShaderObject::setData");
        RHI_RECORD_INPUT_POD(offset);
        RHI_RECORD_INPUT_BLOB(data, size);
        auto result = baseObject->setData(offset, data, size);
        RHI_RECORD_RETURN(result);
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL reserveData(const ShaderOffset& offset, size_t size, void** outData) override
    {
        // Forward to inner object. The caller will write data through
        // the returned pointer; we can't intercept those writes.
        // If replay needs this data, callers should use setData() instead.
        RHI_RECORD_CALL("IShaderObject::reserveData");
        RHI_RECORD_INPUT_POD(offset);
        RHI_RECORD_INPUT_POD(size);
        auto result = baseObject->reserveData(offset, size, outData);
        RHI_RECORD_RETURN(result);
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL getObject(const ShaderOffset& offset, IShaderObject** object) override
    {
        ComPtr<IShaderObject> innerObj;
        SLANG_RETURN_ON_FAIL(baseObject->getObject(offset, innerObj.writeRef()));
        if (!innerObj)
        {
            *object = nullptr;
            return SLANG_OK;
        }

        uint64_t key = (uint64_t(offset.bindingRangeIndex) << 32) | offset.bindingArrayIndex;
        auto it = m_childObjects.find(key);
        if (it != m_childObjects.end() && it->second->baseObject == innerObj)
        {
            *object = it->second;
            it->second->addRef();
            return SLANG_OK;
        }

        RefPtr<RecordShaderObject> wrapped = new RecordShaderObject();
        wrapped->baseObject = innerObj;
        wrapped->registerSelf();
        m_childObjects[key] = ComPtr<RecordShaderObject>(wrapped.get());
        returnComPtr(object, wrapped);
        return SLANG_OK;
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL setObject(const ShaderOffset& offset, IShaderObject* object) override
    {
        RHI_RECORD_CALL("IShaderObject::setObject");
        RHI_RECORD_INPUT_POD(offset);
        RHI_RECORD_OBJECT_INPUT(object);
        auto result = baseObject->setObject(offset, getInnerObj(object));
        RHI_RECORD_RETURN(result);
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL setBinding(const ShaderOffset& offset, const Binding& binding) override
    {
        RHI_RECORD_CALL("IShaderObject::setBinding");
        RHI_RECORD_INPUT_POD(offset);
        RHI_RECORD_INPUT_POD(binding);

        Binding innerBinding = binding;
        if (innerBinding.resource)
            innerBinding.resource = getInnerResource(innerBinding.resource);
        if (innerBinding.resource2)
            innerBinding.resource2 = getInnerResource(innerBinding.resource2);

        auto result = baseObject->setBinding(offset, innerBinding);
        RHI_RECORD_RETURN(result);
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL setDescriptorHandle(const ShaderOffset& offset, const DescriptorHandle& handle) override
    {
        RHI_RECORD_CALL("IShaderObject::setDescriptorHandle");
        RHI_RECORD_INPUT_POD(offset);
        RHI_RECORD_INPUT_POD(handle);
        auto result = baseObject->setDescriptorHandle(offset, handle);
        RHI_RECORD_RETURN(result);
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL setSpecializationArgs(const ShaderOffset& offset, const slang::SpecializationArg* args, uint32_t count) override
    {
        RHI_RECORD_CALL("IShaderObject::setSpecializationArgs");
        RHI_RECORD_INPUT_POD(offset);
        RHI_RECORD_INPUT_UINT32(count);
        for (uint32_t i = 0; i < count; i++)
            RHI_RECORD_INPUT_POD(args[i]);
        auto result = baseObject->setSpecializationArgs(offset, args, count);
        RHI_RECORD_RETURN(result);
    }

    virtual SLANG_NO_THROW const void* SLANG_MCALL getRawData() override
    {
        return baseObject->getRawData();
    }

    virtual SLANG_NO_THROW size_t SLANG_MCALL getSize() override
    {
        return baseObject->getSize();
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL setConstantBufferOverride(IBuffer* constantBuffer) override
    {
        RHI_RECORD_CALL("IShaderObject::setConstantBufferOverride");
        RHI_RECORD_OBJECT_INPUT(constantBuffer);
        auto result = baseObject->setConstantBufferOverride(getInnerObj(constantBuffer));
        RHI_RECORD_RETURN(result);
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL finalize() override
    {
        RHI_RECORD_CALL("IShaderObject::finalize");
        auto result = baseObject->finalize();
        RHI_RECORD_RETURN(result);
    }

    virtual SLANG_NO_THROW bool SLANG_MCALL isFinalized() override
    {
        return baseObject->isFinalized();
    }

    void reset()
    {
        m_entryPoints.clear();
        m_childObjects.clear();
        baseObject.setNull();
    }

private:
    std::vector<ComPtr<RecordShaderObject>> m_entryPoints;
    std::unordered_map<uint64_t, ComPtr<RecordShaderObject>> m_childObjects;
};

} // namespace rhi::record
