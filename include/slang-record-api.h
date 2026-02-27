#pragma once

/// @file slang-record-api.h
/// Public recording API for external layers (e.g., slang-rhi) to feed calls
/// into slang's replay system without depending on slang's internal headers.
///
/// All functions operate on the process-wide ReplayContext singleton.
/// When recording is inactive, each function is a fast no-op.

#include <slang.h>

#include <cstddef>
#include <cstdint>

#if defined(__cplusplus)
extern "C"
{
#endif

// ============================================================================
// Recording flags (matches internal RecordFlag values)
// ============================================================================

enum SlangRecordFlag : uint32_t
{
    SLANG_RECORD_FLAG_INPUT = 1,
    SLANG_RECORD_FLAG_OUTPUT = 2,
    SLANG_RECORD_FLAG_RETURN_VALUE = 4,
};

// ============================================================================
// State queries
// ============================================================================

/// Returns true if the recording system is active (writing to a stream).
SLANG_API bool slangRecord_isActive();

/// Returns true if the recording system is actively recording (not replaying).
/// Use this when you only want to activate recording proxies during capture,
/// not during playback.
SLANG_API bool slangRecord_isRecording();

// ============================================================================
// Locking
// ============================================================================

/// Acquire the recording mutex. Returns an opaque handle.
/// Call slangRecord_releaseLock() when done.
/// Returns nullptr if recording is not active (no lock taken).
SLANG_API void* slangRecord_acquireLock();

/// Release a lock previously acquired by slangRecord_acquireLock().
SLANG_API void slangRecord_releaseLock(void* lockHandle);

// ============================================================================
// Call recording
// ============================================================================

/// Begin recording a method call.
/// @param signature  Pre-normalized signature string (e.g., "IDevice::createTexture").
/// @param thisObj    The COM proxy object that 'this' points to.
///                   Its handle is looked up in the shared handle table.
SLANG_API void slangRecord_beginCall(const char* signature, ISlangUnknown* thisObj);

/// Begin recording a static/free function call (no 'this' pointer).
/// @param signature  Pre-normalized signature string (e.g., "rhi::createDevice").
SLANG_API void slangRecord_beginStaticCall(const char* signature);

// ============================================================================
// Primitive value recording
// ============================================================================

SLANG_API void slangRecord_recordBool(uint32_t flags, bool value);
SLANG_API void slangRecord_recordInt32(uint32_t flags, int32_t value);
SLANG_API void slangRecord_recordUInt32(uint32_t flags, uint32_t value);
SLANG_API void slangRecord_recordInt64(uint32_t flags, int64_t value);
SLANG_API void slangRecord_recordUInt64(uint32_t flags, uint64_t value);
SLANG_API void slangRecord_recordFloat(uint32_t flags, float value);
SLANG_API void slangRecord_recordDouble(uint32_t flags, double value);
SLANG_API void slangRecord_recordString(uint32_t flags, const char* value);

// ============================================================================
// POD struct recording
// ============================================================================

/// Record an arbitrary POD struct as size-prefixed raw binary data.
/// @param flags  Recording flags (input/output/return).
/// @param data   Pointer to the struct.
/// @param size   Size of the struct in bytes.
SLANG_API void slangRecord_recordPOD(uint32_t flags, const void* data, uint32_t size);

// ============================================================================
// Blob recording (content-addressed)
// ============================================================================

/// Record raw data as a content-addressed blob (hashed, stored to disk).
/// Pass data=nullptr or size=0 to record a null blob.
SLANG_API void slangRecord_recordBlob(uint32_t flags, const void* data, size_t size);

// ============================================================================
// COM object handle recording
// ============================================================================

/// Record an object handle. Looks up the object in the shared handle table.
/// Pass nullptr to record a null handle.
SLANG_API void slangRecord_recordHandle(uint32_t flags, ISlangUnknown* obj);

// ============================================================================
// Proxy management
// ============================================================================

/// Register a proxy-implementation pair in the shared handle table.
/// Call this after creating a recording proxy that wraps a real object.
/// Returns the assigned handle ID.
SLANG_API uint64_t slangRecord_registerProxy(
    ISlangUnknown* proxy,
    ISlangUnknown* implementation);

/// Unregister a proxy from the handle table.
/// Call this from the proxy destructor.
/// @param proxyIdentity  The canonical ISlangUnknown* identity of the proxy.
///                       Must be computed before ref count reaches 0 (e.g., cached
///                       at construction time), since queryInterface is unsafe in
///                       destructors.
SLANG_API void slangRecord_unregisterProxy(ISlangUnknown* proxyIdentity);

/// Get the handle ID for a registered proxy object.
/// Returns 0 (null handle) if the object is not registered.
SLANG_API uint64_t slangRecord_getProxyHandle(ISlangUnknown* obj);

#if defined(__cplusplus)
} // extern "C"

/// RAII lock guard for the recording mutex.
/// When recording is inactive, the lock is a no-op (no mutex taken).
class SlangRecordLock
{
    void* m_handle;

public:
    SlangRecordLock() : m_handle(slangRecord_acquireLock()) {}
    ~SlangRecordLock()
    {
        if (m_handle)
            slangRecord_releaseLock(m_handle);
    }
    SlangRecordLock(const SlangRecordLock&) = delete;
    SlangRecordLock& operator=(const SlangRecordLock&) = delete;
};

#endif
