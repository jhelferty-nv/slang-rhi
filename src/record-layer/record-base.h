#pragma once

#include <slang-com-ptr.h>
#include <slang-record-api.h>
#include <slang-rhi.h>

#include "core/common.h"

namespace rhi::record {

using Slang::ComPtr;

// =============================================================================
// Recording macros
// =============================================================================

/// Begin a recorded instance method call with an explicit signature string.
/// The signature should match the public interface name (e.g., "IDevice::createTexture").
/// Sets up `_lock` local used for thread safety.
#define RHI_RECORD_CALL(sig)                                                       \
    SlangRecordLock _lock;                                                         \
    slangRecord_beginCall(sig, static_cast<ISlangUnknown*>(this))

/// Record an input primitive that has a matching slangRecord_recordXxx function.
/// Use for int32_t, uint32_t, int64_t, uint64_t, bool, float, double.
#define RHI_RECORD_INPUT_INT32(arg) slangRecord_recordInt32(SLANG_RECORD_FLAG_INPUT, arg)
#define RHI_RECORD_INPUT_UINT32(arg) slangRecord_recordUInt32(SLANG_RECORD_FLAG_INPUT, arg)
#define RHI_RECORD_INPUT_INT64(arg) slangRecord_recordInt64(SLANG_RECORD_FLAG_INPUT, arg)
#define RHI_RECORD_INPUT_UINT64(arg) slangRecord_recordUInt64(SLANG_RECORD_FLAG_INPUT, arg)
#define RHI_RECORD_INPUT_BOOL(arg) slangRecord_recordBool(SLANG_RECORD_FLAG_INPUT, arg)

/// Record an input POD struct (e.g., desc structs) as raw binary data.
#define RHI_RECORD_INPUT_POD(arg) \
    slangRecord_recordPOD(SLANG_RECORD_FLAG_INPUT, &(arg), static_cast<uint32_t>(sizeof(arg)))

/// Record an input array of POD data.
#define RHI_RECORD_INPUT_BLOB(data, size) \
    slangRecord_recordBlob(SLANG_RECORD_FLAG_INPUT, data, size)

/// Record a counted array of POD elements.
#define RHI_RECORD_INPUT_POD_ARRAY(arr, count)                                 \
    do {                                                                       \
        uint32_t _n = static_cast<uint32_t>(count);                            \
        RHI_RECORD_INPUT_UINT32(_n);                                           \
        if ((arr) && _n > 0)                                                   \
            slangRecord_recordPOD(SLANG_RECORD_FLAG_INPUT, (arr),              \
                static_cast<uint32_t>(sizeof((arr)[0]) * _n));                 \
    } while (0)

/// Record a counted array of C strings.
#define RHI_RECORD_INPUT_STRING_ARRAY(arr, count)                              \
    do {                                                                       \
        uint32_t _n = static_cast<uint32_t>(count);                            \
        RHI_RECORD_INPUT_UINT32(_n);                                           \
        for (uint32_t _i = 0; _i < _n; _i++)                                  \
            slangRecord_recordString(SLANG_RECORD_FLAG_INPUT, (arr)[_i]);      \
    } while (0)

/// Record an output POD value.
#define RHI_RECORD_OUTPUT(arg) \
    slangRecord_recordPOD(SLANG_RECORD_FLAG_OUTPUT, &(arg), static_cast<uint32_t>(sizeof(arg)))

/// Record an RHI COM interface pointer as input.
#define RHI_RECORD_OBJECT_INPUT(arg) \
    slangRecord_recordHandle(SLANG_RECORD_FLAG_INPUT, static_cast<ISlangUnknown*>(arg))

/// Record an RHI COM output (T** style - dereference then record handle).
#define RHI_RECORD_OBJECT_OUTPUT(arg) \
    slangRecord_recordHandle(                                                      \
        SLANG_RECORD_FLAG_OUTPUT,                                                  \
        (arg) && *(arg) ? static_cast<ISlangUnknown*>(*(arg)) : nullptr)

/// Record return value and return it.
#define RHI_RECORD_RETURN(result)                                                  \
    slangRecord_recordInt32(SLANG_RECORD_FLAG_RETURN_VALUE, result);               \
    return result

/// Record void return.
#define RHI_RECORD_RETURN_VOID() ((void)0)

/// Create a temp local if the pointer output is null (so we still record the output).
#define RHI_PREPARE_OUTPUT(arg)                                                    \
    std::decay_t<decltype(*arg)> _temp_##arg{};                                    \
    if (!arg)                                                                      \
        arg = &_temp_##arg

// =============================================================================
// Extension chain serialization
// =============================================================================

/// Common header for all extensible descriptor structs.
struct ExtensibleStructHeader
{
    StructType structType;
    const void* next;
};

/// Walk and serialize an extension chain starting at `next`.
/// Each node is identified by its StructType discriminant, serialized as POD,
/// and then we follow its `next` pointer. A sentinel StructType marks the end.
inline void recordExtensionChain(const void* next)
{
    constexpr uint32_t kEndOfChain = 0xFFFFFFFF;

    while (next)
    {
        auto* header = static_cast<const ExtensibleStructHeader*>(next);
        uint32_t type = static_cast<uint32_t>(header->structType);
        slangRecord_recordUInt32(SLANG_RECORD_FLAG_INPUT, type);

        switch (header->structType)
        {
        case StructType::D3D12DeviceExtendedDesc:
            slangRecord_recordPOD(SLANG_RECORD_FLAG_INPUT, next, sizeof(D3D12DeviceExtendedDesc));
            break;
        case StructType::D3D12ExperimentalFeaturesDesc:
            slangRecord_recordPOD(SLANG_RECORD_FLAG_INPUT, next, sizeof(D3D12ExperimentalFeaturesDesc));
            break;
        case StructType::VulkanDeviceExtendedDesc:
            slangRecord_recordPOD(SLANG_RECORD_FLAG_INPUT, next, sizeof(VulkanDeviceExtendedDesc));
            break;
        default:
            break;
        }

        next = header->next;
    }

    slangRecord_recordUInt32(SLANG_RECORD_FLAG_INPUT, kEndOfChain);
}

/// Record a descriptor struct as POD followed by its extension chain.
#define RHI_RECORD_INPUT_DESC(arg)                                             \
    RHI_RECORD_INPUT_POD(arg);                                                 \
    recordExtensionChain((arg).next)

// =============================================================================
// Base classes
// =============================================================================

/// Base class for all RHI recording proxies (owned, reference-counted).
/// Mirrors the debug layer's DebugObject<T> pattern but registers with
/// slang's recording system for handle-based recording.
template<typename TInterface>
class RecordObject : public TInterface, public ComObject
{
public:
    SLANG_COM_OBJECT_IUNKNOWN_ALL;
    TInterface* getInterface(const Guid& guid)
    {
        if (guid == ISlangUnknown::getTypeGuid() || guid == TInterface::getTypeGuid())
            return static_cast<TInterface*>(this);
        return nullptr;
    }

    ComPtr<TInterface> baseObject;

    RecordObject()
    {
        m_cachedIdentity = static_cast<ISlangUnknown*>(static_cast<TInterface*>(this));
    }

    virtual ~RecordObject()
    {
        if (slangRecord_isActive() && m_cachedIdentity)
            slangRecord_unregisterProxy(m_cachedIdentity);
    }

    void registerSelf()
    {
        if (slangRecord_isActive())
            slangRecord_registerProxy(
                static_cast<ISlangUnknown*>(static_cast<TInterface*>(this)),
                static_cast<ISlangUnknown*>(static_cast<TInterface*>(baseObject.get())));
    }

private:
    ISlangUnknown* m_cachedIdentity = nullptr;
};

/// Base class for unowned RHI recording proxies (pass encoders).
/// Lifetime is tied to the parent RecordCommandEncoder.
template<typename TInterface>
class UnownedRecordObject : public TInterface, public ComObject
{
public:
    TInterface* getInterface(const Guid& guid)
    {
        if (guid == ISlangUnknown::getTypeGuid() || guid == TInterface::getTypeGuid())
            return static_cast<TInterface*>(this);
        return nullptr;
    }

    TInterface* baseObject = nullptr;

    virtual SLANG_NO_THROW uint32_t SLANG_MCALL addRef() override { return 1; }
    virtual SLANG_NO_THROW uint32_t SLANG_MCALL release() override { return 1; }
    virtual SLANG_NO_THROW Result SLANG_MCALL queryInterface(SlangUUID const& uuid, void** outObject) noexcept override
    {
        void* intf = getInterface(uuid);
        if (intf)
        {
            *outObject = intf;
            return SLANG_OK;
        }
        return SLANG_E_NO_INTERFACE;
    }

    void registerSelf()
    {
        if (slangRecord_isActive() && baseObject)
            slangRecord_registerProxy(
                static_cast<ISlangUnknown*>(static_cast<TInterface*>(this)),
                static_cast<ISlangUnknown*>(baseObject));
    }
};

// =============================================================================
// getInnerObj helpers — per-type overloads following the debug layer pattern
// =============================================================================

#define SLANG_RHI_RECORD_GET_OBJ_IMPL(type)                                   \
    inline I##type* getInnerObj(I##type* ptr)                                  \
    {                                                                          \
        if (!ptr)                                                              \
            return nullptr;                                                    \
        return checked_cast<RecordObject<I##type>*>(ptr)->baseObject.get();    \
    }

#define SLANG_RHI_RECORD_GET_OBJ_IMPL_UNOWNED(type)                           \
    inline I##type* getInnerObj(I##type* ptr)                                  \
    {                                                                          \
        if (!ptr)                                                              \
            return nullptr;                                                    \
        return checked_cast<UnownedRecordObject<I##type>*>(ptr)->baseObject;   \
    }

SLANG_RHI_RECORD_GET_OBJ_IMPL(Device)
SLANG_RHI_RECORD_GET_OBJ_IMPL(Buffer)
SLANG_RHI_RECORD_GET_OBJ_IMPL(Texture)
SLANG_RHI_RECORD_GET_OBJ_IMPL(TextureView)
SLANG_RHI_RECORD_GET_OBJ_IMPL(Sampler)
SLANG_RHI_RECORD_GET_OBJ_IMPL(AccelerationStructure)
SLANG_RHI_RECORD_GET_OBJ_IMPL(ShaderObject)
SLANG_RHI_RECORD_GET_OBJ_IMPL(ShaderProgram)
SLANG_RHI_RECORD_GET_OBJ_IMPL(RenderPipeline)
SLANG_RHI_RECORD_GET_OBJ_IMPL(ComputePipeline)
SLANG_RHI_RECORD_GET_OBJ_IMPL(RayTracingPipeline)
SLANG_RHI_RECORD_GET_OBJ_IMPL(InputLayout)
SLANG_RHI_RECORD_GET_OBJ_IMPL(ShaderTable)
SLANG_RHI_RECORD_GET_OBJ_IMPL(CommandBuffer)
SLANG_RHI_RECORD_GET_OBJ_IMPL(CommandQueue)
SLANG_RHI_RECORD_GET_OBJ_IMPL(CommandEncoder)
SLANG_RHI_RECORD_GET_OBJ_IMPL(Fence)
SLANG_RHI_RECORD_GET_OBJ_IMPL(QueryPool)
SLANG_RHI_RECORD_GET_OBJ_IMPL(Surface)
SLANG_RHI_RECORD_GET_OBJ_IMPL(Heap)

/// Helper to unwrap a BufferOffsetPair in-place.
inline BufferOffsetPair getInnerBufferOffsetPair(const BufferOffsetPair& pair)
{
    return BufferOffsetPair(getInnerObj(pair.buffer), pair.offset);
}

/// Unwrap an IResource* by trying known resource types.
/// IResource is a base interface for IBuffer, ITexture, ITextureView, ISampler, IAccelerationStructure.
inline IResource* getInnerResource(IResource* ptr)
{
    if (!ptr)
        return nullptr;

    // Try each concrete resource type. The proxy's queryInterface returns the proxy
    // itself for matching GUIDs, so we can determine the type.
    {
        ComPtr<IBuffer> buf;
        if (SLANG_SUCCEEDED(ptr->queryInterface(IBuffer::getTypeGuid(), (void**)buf.writeRef())) && buf)
            return getInnerObj(buf.get());
    }
    {
        ComPtr<ITextureView> view;
        if (SLANG_SUCCEEDED(ptr->queryInterface(ITextureView::getTypeGuid(), (void**)view.writeRef())) && view)
            return getInnerObj(view.get());
    }
    {
        ComPtr<ITexture> tex;
        if (SLANG_SUCCEEDED(ptr->queryInterface(ITexture::getTypeGuid(), (void**)tex.writeRef())) && tex)
            return getInnerObj(tex.get());
    }
    {
        ComPtr<ISampler> sam;
        if (SLANG_SUCCEEDED(ptr->queryInterface(ISampler::getTypeGuid(), (void**)sam.writeRef())) && sam)
            return getInnerObj(sam.get());
    }
    {
        ComPtr<IAccelerationStructure> as;
        if (SLANG_SUCCEEDED(ptr->queryInterface(IAccelerationStructure::getTypeGuid(), (void**)as.writeRef())) && as)
            return getInnerObj(as.get());
    }

    return ptr;
}

} // namespace rhi::record
