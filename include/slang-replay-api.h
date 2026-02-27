#pragma once

/// @file slang-replay-api.h
/// Public replay API for external layers (e.g., slang-rhi) to read values from
/// the replay stream and manage the shared handle table during playback.
///
/// All functions operate on the process-wide ReplayContext singleton.
/// They are only valid during playback mode (when executeNextCall() is running).

#include <slang.h>

#include <cstddef>
#include <cstdint>

#if defined(__cplusplus)
extern "C"
{
#endif

// ============================================================================
// External handler registration
// ============================================================================

/// Callback type for handling unknown call signatures during replay.
/// The handler receives the signature string and should:
///   1. Check if it recognizes the signature
///   2. If yes, read remaining arguments from the stream via slangReplay_readXxx(),
///      execute the call, map any output handles, and return true
///   3. If no, return false (the replay system will throw an error)
///
/// When the handler is invoked, the signature and 'this' handle have already been
/// consumed from the stream. The 'this' handle is available via
/// slangReplay_getCurrentThisHandle().
typedef bool (*SlangReplayExternalHandler)(const char* signature);

/// Register an external handler for call signatures not handled by the Slang
/// replay system. Typically called by the replay tool after linking with slang-rhi.
SLANG_API void slangReplay_setExternalHandler(SlangReplayExternalHandler handler);

// ============================================================================
// Current call context
// ============================================================================

/// Get the 'this' handle for the currently executing call.
/// Valid only during handler execution (inside an external handler callback).
SLANG_API uint64_t slangReplay_getCurrentThisHandle();

// ============================================================================
// Stream reading - primitives
// ============================================================================

/// Each read function consumes a TypeId prefix byte and the value from the stream.
/// Throws TypeMismatchException if the TypeId doesn't match the expected type.

SLANG_API bool slangReplay_readBool();
SLANG_API int32_t slangReplay_readInt32();
SLANG_API uint32_t slangReplay_readUInt32();
SLANG_API int64_t slangReplay_readInt64();
SLANG_API uint64_t slangReplay_readUInt64();
SLANG_API float slangReplay_readFloat();
SLANG_API double slangReplay_readDouble();

/// Read a string from the stream. Returns nullptr for null strings.
/// The returned pointer is valid until the next read operation.
SLANG_API const char* slangReplay_readString();

// ============================================================================
// Stream reading - POD structs
// ============================================================================

/// Read a POD struct from the stream. Reads the size prefix (uint32_t) then
/// raw bytes. The caller must provide a buffer of the correct size.
SLANG_API void slangReplay_readPOD(void* outData, uint32_t expectedSize);

// ============================================================================
// Stream reading - blobs
// ============================================================================

/// Read a content-addressed blob from the stream. Loads the blob data from
/// the files/ directory using the recorded hash.
/// Returns the blob interface; caller owns the reference. Returns nullptr for null blobs.
SLANG_API ISlangBlob* slangReplay_readBlob();

// ============================================================================
// Stream reading - handles
// ============================================================================

/// Read an object handle from the stream and return the raw handle ID.
/// Use slangReplay_getObjectFromHandle() to look up the live object.
SLANG_API uint64_t slangReplay_readHandle();

// ============================================================================
// Handle table management
// ============================================================================

/// Register a newly-created live object with its recorded handle ID.
/// Call this after creating an object during replay to establish the mapping.
SLANG_API void slangReplay_mapHandleToObject(uint64_t handle, ISlangUnknown* obj);

/// Look up a live object by its recorded handle ID.
/// Returns nullptr for handle 0 (null handle).
/// Throws HandleNotFoundException if the handle is not registered.
SLANG_API ISlangUnknown* slangReplay_getObjectFromHandle(uint64_t handle);

#if defined(__cplusplus)
} // extern "C"
#endif
