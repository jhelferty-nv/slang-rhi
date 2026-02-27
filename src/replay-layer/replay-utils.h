#pragma once

/// @file replay-utils.h
/// Deserialization helpers for the RHI replay layer.
/// These mirror the recording macros/helpers in record-base.h, reading data
/// in the exact format that the recording layer wrote.

#include <slang-replay-api.h>
#include <slang-rhi.h>

#include <cstring>
#include <vector>

namespace rhi::replay {

// =============================================================================
// Typed handle lookups
// =============================================================================

template<typename T>
inline T* getObj(uint64_t handle)
{
    if (handle == 0)
        return nullptr;
    return static_cast<T*>(slangReplay_getObjectFromHandle(handle));
}

template<typename T>
inline T* readObjFromStream()
{
    uint64_t handle = slangReplay_readHandle();
    return getObj<T>(handle);
}

// =============================================================================
// POD reading
// =============================================================================

template<typename T>
inline T readPOD()
{
    T value{};
    slangReplay_readPOD(&value, static_cast<uint32_t>(sizeof(T)));
    return value;
}

// =============================================================================
// Extension chain reading (consume and discard)
// =============================================================================

inline void skipExtensionChain()
{
    constexpr uint32_t kEndOfChain = 0xFFFFFFFF;
    for (;;)
    {
        uint32_t structType = slangReplay_readUInt32();
        if (structType == kEndOfChain)
            break;

        auto st = static_cast<StructType>(structType);
        switch (st)
        {
        case StructType::D3D12DeviceExtendedDesc:
            readPOD<D3D12DeviceExtendedDesc>();
            break;
        case StructType::D3D12ExperimentalFeaturesDesc:
            readPOD<D3D12ExperimentalFeaturesDesc>();
            break;
        case StructType::VulkanDeviceExtendedDesc:
            readPOD<VulkanDeviceExtendedDesc>();
            break;
        default:
            break;
        }
    }
}

/// Read a descriptor struct (POD) and consume its extension chain.
/// Sets desc.next = nullptr since the original pointer is invalid during replay.
template<typename T>
inline T readDesc()
{
    T desc = readPOD<T>();
    skipExtensionChain();
    desc.next = nullptr;
    return desc;
}

// =============================================================================
// Array reading helpers
// =============================================================================

template<typename T>
inline std::vector<T> readPODArray()
{
    uint32_t count = slangReplay_readUInt32();
    std::vector<T> arr(count);
    if (count > 0)
        slangReplay_readPOD(arr.data(), static_cast<uint32_t>(sizeof(T) * count));
    return arr;
}

inline std::vector<const char*> readStringArray()
{
    uint32_t count = slangReplay_readUInt32();
    std::vector<const char*> arr(count);
    for (uint32_t i = 0; i < count; i++)
        arr[i] = slangReplay_readString();
    return arr;
}

// =============================================================================
// Return value handling
// =============================================================================

inline int32_t readReturnValue()
{
    return slangReplay_readInt32();
}

inline uint64_t readOutputHandle()
{
    return slangReplay_readHandle();
}

/// Read the output handle from the stream and map it to the given live object.
inline void mapOutputObject(ISlangUnknown* obj)
{
    uint64_t handle = readOutputHandle();
    if (obj && handle != 0)
        slangReplay_mapHandleToObject(handle, obj);
}

} // namespace rhi::replay
