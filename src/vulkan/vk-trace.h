#pragma once

#include <cstddef>

namespace rhi::vk {

/// Vulkan API call tracing for debugging driver crashes.
/// Enable by setting environment variable SLANG_RHI_VK_TRACE to a directory path.
/// Writes slang_rhi_vk_trace.txt (call log) and spirv_dump_*.spv (SPIR-V binaries) there.
/// Flushes to disk before each Vulkan call so logs survive a driver crash.
struct VulkanTrace
{
    /// Initialize trace from SLANG_RHI_VK_TRACE env var. Call once at device init.
    static void init();

    /// Ensure init has been attempted (for lazy init when first Vulkan call runs in this module).
    static void ensureInit();

    /// Returns true if tracing is enabled (env was set and init succeeded).
    static bool isEnabled();

    /// Flush text log to disk. Call before every Vulkan API call when tracing.
    static void flush();

    /// Log a line to the text file (printf-style). Does not flush; call flush() before the next Vulkan call.
    static void log(const char* fmt, ...);

    /// Current thread id (platform-specific, stable for thread lifetime). For trace diagnostics.
    static unsigned long long getThreadId();

    /// Write SPIR-V binary to spirv_dump_<N>.spv, log the filename to the text log, then flush both.
    /// Returns the filename written (or empty if not enabled / error). Call before vkCreateShaderModule.
    static void dumpSpirvAndLog(const void* code, size_t codeSize);
};

/// Call before every Vulkan API invocation: flushes previous log, logs this call, flushes again, then proceed with the call.
/// Uses lazy init so the module that actually makes Vulkan calls opens the log (fixes multi-DLL case).
#define SLANG_VK_TRACE_BEFORE(fmt, ...)                                                                                 \
    do                                                                                                                 \
    {                                                                                                                  \
        rhi::vk::VulkanTrace::ensureInit();                                                                            \
        if (rhi::vk::VulkanTrace::isEnabled())                                                                          \
        {                                                                                                              \
            rhi::vk::VulkanTrace::flush();                                                                             \
            rhi::vk::VulkanTrace::log("[tid=%llu] " fmt "\n", rhi::vk::VulkanTrace::getThreadId(), ##__VA_ARGS__);     \
            rhi::vk::VulkanTrace::flush();                                                                             \
        }                                                                                                              \
    } while (0)

} // namespace rhi::vk
