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

    /// Returns true if tracing is enabled (env was set and init succeeded).
    static bool isEnabled();

    /// Flush text log to disk. Call before every Vulkan API call when tracing.
    static void flush();

    /// Log a line to the text file (printf-style). Does not flush; call flush() before the next Vulkan call.
    static void log(const char* fmt, ...);

    /// Write SPIR-V binary to spirv_dump_<N>.spv, log the filename to the text log, then flush both.
    /// Returns the filename written (or empty if not enabled / error). Call before vkCreateShaderModule.
    static void dumpSpirvAndLog(const void* code, size_t codeSize);
};

/// Call before every Vulkan API invocation: flushes and logs the message, then proceed with the call.
#define SLANG_VK_TRACE_BEFORE(fmt, ...)                                                                                 \
    do                                                                                                                 \
    {                                                                                                                  \
        if (rhi::vk::VulkanTrace::isEnabled())                                                                          \
        {                                                                                                              \
            rhi::vk::VulkanTrace::flush();                                                                             \
            rhi::vk::VulkanTrace::log(fmt "\n", ##__VA_ARGS__);                                                        \
        }                                                                                                              \
    } while (0)

} // namespace rhi::vk
