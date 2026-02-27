#include "vk-trace.h"

#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstring>

#include <stdlib.h>

#if defined(_WIN32)
#include <io.h>
#include <process.h>
#define SLANG_RHI_GETPID() ((unsigned long long)_getpid())
#else
#include <unistd.h>
#define SLANG_RHI_GETPID() ((unsigned long long)getpid())
#endif

namespace rhi::vk {

static FILE* s_logFile = nullptr;
static char s_basePath[1024] = {};
static unsigned long long s_pid = 0;
static uint32_t s_spirvDumpCounter = 0;
static bool s_enabled = false;
static bool s_initAttempted = false;

void VulkanTrace::init()
{
    const char* dir = nullptr;
#if defined(_MSC_VER)
    char envBuf[1024];
    size_t envLen = 0;
    if (getenv_s(&envLen, envBuf, sizeof(envBuf), "SLANG_RHI_VK_TRACE") != 0 || envLen == 0)
        return;
    dir = envBuf;
#else
    dir = getenv("SLANG_RHI_VK_TRACE");
#endif
    if (!dir || dir[0] == '\0')
        return;

    size_t len = strlen(dir);
    if (len >= sizeof(s_basePath) - 64)
        return;

    memcpy(s_basePath, dir, len + 1);
    // Normalize: remove trailing slash
    while (len > 0 && (s_basePath[len - 1] == '/' || s_basePath[len - 1] == '\\'))
        s_basePath[--len] = '\0';

    s_pid = SLANG_RHI_GETPID();
    char logPath[1024];
    snprintf(logPath, sizeof(logPath), "%s/slang_rhi_vk_trace_%llu.txt", s_basePath, s_pid);

#if defined(_MSC_VER)
    if (fopen_s(&s_logFile, logPath, "w") != 0)
        s_logFile = nullptr;
#else
    s_logFile = fopen(logPath, "w");
#endif
    if (!s_logFile)
        return;

    s_enabled = true;
    log("=== Slang-RHI Vulkan trace started ===\n");
    flush();
    // Diagnostic: confirm which module opened the log (helps when only header appeared before)
    fprintf(stderr, "[SLANG_RHI_VK_TRACE] Logging to %s/slang_rhi_vk_trace_%llu.txt\n", s_basePath, s_pid);
    fflush(stderr);
}

void VulkanTrace::ensureInit()
{
    if (!s_initAttempted)
    {
        s_initAttempted = true;
        init();
    }
}

bool VulkanTrace::isEnabled() { return s_enabled && s_logFile != nullptr; }

void VulkanTrace::flush()
{
    if (s_logFile)
    {
        fflush(s_logFile);
#if SLANG_WINDOWS_FAMILY
        _commit(_fileno(s_logFile));
#endif
    }
}

void VulkanTrace::log(const char* fmt, ...)
{
    if (!s_logFile)
        return;
    va_list args;
    va_start(args, fmt);
    vfprintf(s_logFile, fmt, args);
    va_end(args);
}

void VulkanTrace::dumpSpirvAndLog(const void* code, size_t codeSize)
{
    ensureInit();
    if (!s_enabled || !s_logFile || !code || codeSize == 0)
        return;

    uint32_t index = s_spirvDumpCounter++;
    char filename[256];
    snprintf(filename, sizeof(filename), "spirv_dump_%llu_%u.spv", s_pid, index);

    char fullPath[1024];
    snprintf(fullPath, sizeof(fullPath), "%s/%s", s_basePath, filename);

    FILE* bin = nullptr;
#if defined(_MSC_VER)
    if (fopen_s(&bin, fullPath, "wb") != 0)
        bin = nullptr;
#else
    bin = fopen(fullPath, "wb");
#endif
    if (bin)
    {
        size_t written = fwrite(code, 1, codeSize, bin);
        fflush(bin);
#if SLANG_WINDOWS_FAMILY
        _commit(_fileno(bin));
#endif
        fclose(bin);
        log("SPIR-V binary dumped to: %s (%zu bytes written)\n", fullPath, written);
    }
    else
    {
        log("SPIR-V dump failed to open file: %s\n", fullPath);
    }
    flush();
}

} // namespace rhi::vk
