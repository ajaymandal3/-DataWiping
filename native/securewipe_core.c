#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#include <winbase.h>
#endif

#include "native/methods/methods.h"

typedef struct
{
    const char *key;
    const char *label;
    unsigned int passes;
    unsigned long long durationMs;
} WipeMethodDef;

typedef struct
{
    char opId[64];
    char drive[8];
    char method[32];
    char mode[16];
    unsigned long long totalBytes;
    unsigned long long processedBytes;
    unsigned long long logicalBytesToProcess;
    unsigned long long startedAtMs;
    unsigned long long finishedAtMs;
    unsigned long long durationMs;
    unsigned long long currentOffset;
    unsigned int currentPass;
    int active;
    int cancelled;
    int done;
    int ioError;
    char ioErrorMessage[192];
#ifdef _WIN32
    HANDLE volumeHandle;
#endif
} WipeOp;

static const WipeMethodDef kMethods[] = {
    {"CryptoErase", "Crypto Erase (NIST Purge)", 1U, 12000ULL},
    {"SinglePass", "Single Pass (NIST Clear)", 1U, 15000ULL},
    {"ThreePass", "Three Pass (DoD 5220.22-M)", 3U, 35000ULL},
    {"SevenPass", "Seven Pass (Gutmann-lite)", 7U, 70000ULL},
    {"SecureFormat", "Secure Format (Zero + Format)", 1U, 20000ULL},
    {"AtaSecureErase", "ATA Secure Erase (SSD Optimized)", 1U, 14000ULL},
    {"FlashOverwrite", "Flash Overwrite (USB/SD Cards)", 2U, 22000ULL},
    {"EncryptionAware", "Encryption Aware Wipe (Mobile)", 1U, 12000ULL},
    {"RaidServerWipe", "RAID/Server Professional Wipe", 3U, 40000ULL}};

static WipeOp g_op = {0};
static unsigned int g_nextId = 1;
static char g_last_certificate[1024] = "{\"ok\":false,\"message\":\"No certificate yet\"}";
static char g_last_log[1024] = "System initialized";

static unsigned long long now_ms(void)
{
#ifdef _WIN32
    return GetTickCount64();
#else
    return 0ULL;
#endif
}

static int equals_ignore_case(const char *a, const char *b)
{
#ifdef _WIN32
    return _stricmp(a ? a : "", b ? b : "") == 0;
#else
    return strcmp(a ? a : "", b ? b : "") == 0;
#endif
}

static void append_text(char *dst, size_t dstCap, const char *src)
{
    size_t used;
    if (!dst || dstCap == 0 || !src)
    {
        return;
    }
    used = strlen(dst);
    if (used >= dstCap - 1)
    {
        return;
    }
    snprintf(dst + used, dstCap - used, "%s", src);
}

static void append_fmt(char *dst, size_t dstCap, const char *fmt, ...)
{
    size_t used;
    va_list args;
    if (!dst || dstCap == 0 || !fmt)
    {
        return;
    }
    used = strlen(dst);
    if (used >= dstCap - 1)
    {
        return;
    }
    va_start(args, fmt);
    vsnprintf(dst + used, dstCap - used, fmt, args);
    va_end(args);
}

static int is_valid_drive(const char *drive)
{
    return drive && strlen(drive) == 2 && drive[1] == ':';
}

static const WipeMethodDef *find_method(const char *methodKey)
{
    size_t i;
    const char *safe = methodKey ? methodKey : "";
    for (i = 0; i < sizeof(kMethods) / sizeof(kMethods[0]); ++i)
    {
        if (equals_ignore_case(safe, kMethods[i].key))
        {
            return &kMethods[i];
        }
    }
    return NULL;
}

static unsigned long long get_drive_total_bytes(const char *drive)
{
#ifdef _WIN32
    char root[4] = {0};
    ULARGE_INTEGER avail;
    ULARGE_INTEGER total;
    ULARGE_INTEGER freeb;
    if (!is_valid_drive(drive))
    {
        return 0ULL;
    }
    root[0] = drive[0];
    root[1] = ':';
    root[2] = '\\';
    root[3] = '\0';
    if (!GetDiskFreeSpaceExA(root, &avail, &total, &freeb))
    {
        return 0ULL;
    }
    return (unsigned long long)total.QuadPart;
#else
    (void)drive;
    return 0ULL;
#endif
}

#ifdef _WIN32
static void close_volume_handle(void)
{
    if (g_op.volumeHandle && g_op.volumeHandle != INVALID_HANDLE_VALUE)
    {
        FlushFileBuffers(g_op.volumeHandle);
        CloseHandle(g_op.volumeHandle);
        g_op.volumeHandle = INVALID_HANDLE_VALUE;
    }
}

static int open_volume_handle(const char *drive, char *errBuf, size_t errCap)
{
    char volumePath[16];
    DWORD bytesReturned = 0;
    snprintf(volumePath, sizeof(volumePath), "\\\\.\\%s", drive);

    g_op.volumeHandle = CreateFileA(
        volumePath,
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL);

    if (g_op.volumeHandle == INVALID_HANDLE_VALUE)
    {
        snprintf(errBuf, errCap, "Failed to open volume %s (winerr=%lu)", drive, (unsigned long)GetLastError());
        return 0;
    }

    DeviceIoControl(g_op.volumeHandle, FSCTL_LOCK_VOLUME, NULL, 0, NULL, 0, &bytesReturned, NULL);
    DeviceIoControl(g_op.volumeHandle, FSCTL_DISMOUNT_VOLUME, NULL, 0, NULL, 0, &bytesReturned, NULL);
    return 1;
}

static unsigned char resolve_pass_pattern(unsigned int passIndex, const char *method)
{
    /* Pattern resolution now handled by modular methods dispatcher (methods/) */
    WipeMethodType methodType = method_from_key(method);
    return get_wipe_pattern(methodType, passIndex);
}

static void set_io_error(const char *msg)
{
    g_op.ioError = 1;
    snprintf(g_op.ioErrorMessage, sizeof(g_op.ioErrorMessage), "%s", msg ? msg : "Unknown I/O error");
}

static int perform_real_wipe_step(const WipeMethodDef *def)
{
    const DWORD chunkSize = 1024U * 1024U;
    BYTE *buffer;
    DWORD written = 0;
    LARGE_INTEGER pos;
    unsigned long long remainingOnPass;
    unsigned long long chunkBytes;
    unsigned char pattern;

    if (!def)
    {
        set_io_error("Internal method lookup failed.");
        return 0;
    }
    if (g_op.volumeHandle == INVALID_HANDLE_VALUE)
    {
        set_io_error("Volume handle is not open.");
        return 0;
    }
    if (g_op.currentPass >= def->passes)
    {
        return 0;
    }

    remainingOnPass = (g_op.totalBytes > g_op.currentOffset) ? (g_op.totalBytes - g_op.currentOffset) : 0ULL;
    if (remainingOnPass == 0ULL)
    {
        g_op.currentPass += 1U;
        g_op.currentOffset = 0ULL;
        return 0;
    }

    chunkBytes = remainingOnPass > (unsigned long long)chunkSize ? (unsigned long long)chunkSize : remainingOnPass;
    pattern = resolve_pass_pattern(g_op.currentPass, g_op.method);

    buffer = (BYTE *)malloc((size_t)chunkBytes);
    if (!buffer)
    {
        set_io_error("Failed to allocate I/O buffer.");
        return 0;
    }
    memset(buffer, pattern, (size_t)chunkBytes);

    pos.QuadPart = (LONGLONG)g_op.currentOffset;
    if (!SetFilePointerEx(g_op.volumeHandle, pos, NULL, FILE_BEGIN))
    {
        free(buffer);
        set_io_error("Failed to seek target volume.");
        return 0;
    }

    if (!WriteFile(g_op.volumeHandle, buffer, (DWORD)chunkBytes, &written, NULL))
    {
        free(buffer);
        set_io_error("Failed to write wipe pattern to volume.");
        return 0;
    }
    free(buffer);

    g_op.currentOffset += (unsigned long long)written;
    g_op.processedBytes = ((unsigned long long)g_op.currentPass * g_op.totalBytes) + g_op.currentOffset;

    if (g_op.currentOffset >= g_op.totalBytes)
    {
        g_op.currentPass += 1U;
        g_op.currentOffset = 0ULL;
        FlushFileBuffers(g_op.volumeHandle);
    }
    return 1;
}
#endif

static void finish_operation(void)
{
    g_op.active = 0;
    g_op.done = 1;
    g_op.finishedAtMs = now_ms();
#ifdef _WIN32
    close_volume_handle();
#endif
}

static void refresh_progress(void)
{
    unsigned long long elapsed;
    unsigned long long duration;
    unsigned long long processed;
    const WipeMethodDef *def = find_method(g_op.method);

    if (!g_op.active)
    {
        return;
    }
    if (g_op.cancelled)
    {
        finish_operation();
        return;
    }
    if (g_op.ioError)
    {
        g_op.cancelled = 1;
        finish_operation();
        return;
    }

    if (equals_ignore_case(g_op.mode, "real"))
    {
#ifdef _WIN32
        unsigned int loops = 0;
        const unsigned int maxLoopsPerTick = 64U; /* ~64MB processed per status call */
        while (loops < maxLoopsPerTick && g_op.active && !g_op.cancelled && !g_op.ioError)
        {
            if (!perform_real_wipe_step(def))
            {
                break;
            }
            loops += 1U;
        }
        if (g_op.ioError)
        {
            g_op.cancelled = 1;
            finish_operation();
            return;
        }
        if (def && g_op.currentPass >= def->passes)
        {
            g_op.processedBytes = g_op.logicalBytesToProcess;
            finish_operation();
        }
#else
        g_op.cancelled = 1;
        finish_operation();
#endif
        return;
    }

    elapsed = now_ms() - g_op.startedAtMs;
    duration = g_op.durationMs ? g_op.durationMs : 1ULL;
    processed = (g_op.logicalBytesToProcess * elapsed) / duration;
    if (processed >= g_op.logicalBytesToProcess)
    {
        processed = g_op.logicalBytesToProcess;
        finish_operation();
    }
    g_op.processedBytes = processed;
}

static const char *resolve_stage(void)
{
    unsigned progress;
    if (g_op.active)
    {
        progress = g_op.logicalBytesToProcess > 0ULL ? (unsigned)((g_op.processedBytes * 100ULL) / g_op.logicalBytesToProcess) : 0U;
        if (equals_ignore_case(g_op.mode, "real"))
        {
            if (equals_ignore_case(g_op.method, "SecureFormat"))
            {
                return progress < 95U ? "overwriting" : "finalizing";
            }
            return "overwriting";
        }
        if (equals_ignore_case(g_op.method, "CryptoErase"))
        {
            return progress < 75U ? "encrypting" : "destroying-key";
        }
        return "simulating";
    }
    if (g_op.done)
    {
        return g_op.cancelled ? "cancelled" : "completed";
    }
    return "starting";
}

const char *securewipe_version(void)
{
    return "SecureWipe C Core (Windows) v3.0";
}

const char *securewipe_get_methods_json(void)
{
    static char out[1024];
    size_t i;
    out[0] = '\0';
    append_text(out, sizeof(out), "[");
    for (i = 0; i < sizeof(kMethods) / sizeof(kMethods[0]); ++i)
    {
        if (i > 0)
        {
            append_text(out, sizeof(out), ",");
        }
        append_fmt(out, sizeof(out), "{\"key\":\"%s\",\"label\":\"%s\",\"passes\":%u}", kMethods[i].key, kMethods[i].label, kMethods[i].passes);
    }
    append_text(out, sizeof(out), "]");
    return out;
}

const char *securewipe_list_disks_json(void)
{
    static char out[8192];
    out[0] = '\0';
#ifdef _WIN32
    DWORD mask = GetLogicalDrives();
    char systemDrive[3] = "C:";
    int first = 1;
    int i;
    GetEnvironmentVariableA("SystemDrive", systemDrive, 3);
    append_text(out, sizeof(out), "[");
    for (i = 0; i < 26; ++i)
    {
        char driveName[3];
        char root[4];
        UINT type;
        ULARGE_INTEGER avail;
        ULARGE_INTEGER total;
        ULARGE_INTEGER freeb;
        int hasSize;
        int isSystem;
        if (!(mask & (1u << i)))
        {
            continue;
        }
        driveName[0] = (char)('A' + i);
        driveName[1] = ':';
        driveName[2] = '\0';
        root[0] = driveName[0];
        root[1] = ':';
        root[2] = '\\';
        root[3] = '\0';
        type = GetDriveTypeA(root);
        if (type == DRIVE_NO_ROOT_DIR)
        {
            continue;
        }
        hasSize = GetDiskFreeSpaceExA(root, &avail, &total, &freeb) ? 1 : 0;
        isSystem = equals_ignore_case(driveName, systemDrive) ? 1 : 0;
        if (!first)
        {
            append_text(out, sizeof(out), ",");
        }
        first = 0;
        append_fmt(out, sizeof(out), "{\"name\":\"%s\",\"type\":%u,\"totalBytes\":%llu,\"freeBytes\":%llu,\"isSystem\":%s}", driveName, (unsigned)type, (unsigned long long)(hasSize ? total.QuadPart : 0ULL), (unsigned long long)(hasSize ? freeb.QuadPart : 0ULL), isSystem ? "true" : "false");
    }
    append_text(out, sizeof(out), "]");
#else
    strcpy(out, "[]");
#endif
    return out;
}

const char *securewipe_start_wipe_json(const char *drive, const char *method, const char *mode)
{
    static char out[768];
    const char *safeDrive = drive ? drive : "";
    const char *safeMethod = method ? method : "SinglePass";
    const char *safeMode = mode ? mode : "real";
    const WipeMethodDef *def;

    if (g_op.active)
    {
        snprintf(out, sizeof(out), "{\"ok\":false,\"code\":\"OP_IN_PROGRESS\",\"message\":\"Another operation is running\",\"opId\":\"%s\"}", g_op.opId);
        return out;
    }
    if (!is_valid_drive(safeDrive))
    {
        snprintf(out, sizeof(out), "{\"ok\":false,\"code\":\"INVALID_DRIVE\",\"message\":\"Drive must look like C:\"}");
        return out;
    }
    if (equals_ignore_case(safeDrive, "C:"))
    {
        snprintf(out, sizeof(out), "{\"ok\":false,\"code\":\"SYSTEM_DRIVE_BLOCKED\",\"message\":\"System drive cannot be wiped\"}");
        return out;
    }
    if (!equals_ignore_case(safeMode, "real") && !equals_ignore_case(safeMode, "simulate"))
    {
        snprintf(out, sizeof(out), "{\"ok\":false,\"code\":\"INVALID_MODE\",\"message\":\"Mode must be 'real' or 'simulate'\"}");
        return out;
    }

    def = find_method(safeMethod);
    if (!def)
    {
        snprintf(out, sizeof(out), "{\"ok\":false,\"code\":\"INVALID_METHOD\",\"message\":\"Unsupported wipe method\"}");
        return out;
    }

    memset(&g_op, 0, sizeof(g_op));
#ifdef _WIN32
    g_op.volumeHandle = INVALID_HANDLE_VALUE;
#endif
    snprintf(g_op.opId, sizeof(g_op.opId), "op-%u", g_nextId++);
    strncpy(g_op.drive, safeDrive, sizeof(g_op.drive) - 1);
    strncpy(g_op.method, def->key, sizeof(g_op.method) - 1);
    strncpy(g_op.mode, safeMode, sizeof(g_op.mode) - 1);
    g_op.totalBytes = get_drive_total_bytes(g_op.drive);
    if (g_op.totalBytes == 0ULL)
    {
        snprintf(out, sizeof(out), "{\"ok\":false,\"code\":\"DRIVE_SIZE_UNAVAILABLE\",\"message\":\"Unable to determine target drive size\"}");
        return out;
    }
    g_op.logicalBytesToProcess = g_op.totalBytes * (unsigned long long)def->passes;
    g_op.startedAtMs = now_ms();
    g_op.durationMs = def->durationMs;
    g_op.active = 1;

    if (equals_ignore_case(g_op.mode, "real"))
    {
#ifdef _WIN32
        if (!open_volume_handle(g_op.drive, g_op.ioErrorMessage, sizeof(g_op.ioErrorMessage)))
        {
            g_op.active = 0;
            snprintf(out, sizeof(out), "{\"ok\":false,\"code\":\"OPEN_FAILED\",\"message\":\"%s\"}", g_op.ioErrorMessage);
            return out;
        }
        snprintf(g_last_log, sizeof(g_last_log), "Started REAL wipe (%s) on %s", g_op.method, g_op.drive);
#else
        snprintf(out, sizeof(out), "{\"ok\":false,\"code\":\"PLATFORM_UNSUPPORTED\",\"message\":\"Real mode is only supported on Windows\"}");
        return out;
#endif
    }
    else
    {
        snprintf(g_last_log, sizeof(g_last_log), "Started SIMULATED wipe (%s) on %s", g_op.method, g_op.drive);
    }

    snprintf(out, sizeof(out), "{\"ok\":true,\"opId\":\"%s\",\"drive\":\"%s\",\"method\":\"%s\",\"mode\":\"%s\",\"message\":\"Wipe operation started\"}", g_op.opId, g_op.drive, g_op.method, g_op.mode);
    return out;
}

const char *securewipe_cancel_wipe_json(const char *opId)
{
    static char out[256];
    const char *safeOpId = opId ? opId : "";
    refresh_progress();
    if (!g_op.active)
    {
        snprintf(out, sizeof(out), "{\"ok\":false,\"code\":\"NOT_RUNNING\",\"message\":\"No active operation\"}");
        return out;
    }
    if (strcmp(g_op.opId, safeOpId) != 0)
    {
        snprintf(out, sizeof(out), "{\"ok\":false,\"code\":\"NOT_FOUND\",\"message\":\"Operation not found\"}");
        return out;
    }
    g_op.cancelled = 1;
    refresh_progress();
    snprintf(g_last_log, sizeof(g_last_log), "Cancel requested for %s", g_op.opId);
    snprintf(out, sizeof(out), "{\"ok\":true,\"opId\":\"%s\",\"message\":\"Cancellation requested\"}", g_op.opId);
    return out;
}

const char *securewipe_get_wipe_status_json(const char *opId)
{
    static char out[1024];
    const char *safeOpId = opId ? opId : "";
    int progress;
    const char *stage;

    refresh_progress();
    if (g_op.opId[0] == '\0')
    {
        snprintf(out, sizeof(out), "{\"ok\":false,\"code\":\"NO_OPERATIONS\",\"message\":\"No operation started\"}");
        return out;
    }
    if (strcmp(safeOpId, g_op.opId) != 0)
    {
        snprintf(out, sizeof(out), "{\"ok\":false,\"code\":\"NOT_FOUND\",\"message\":\"Operation not found\"}");
        return out;
    }
    if (g_op.ioError)
    {
        snprintf(g_last_log, sizeof(g_last_log), "I/O error: %s", g_op.ioErrorMessage);
    }
    else if (g_op.done && !g_op.cancelled)
    {
        snprintf(g_last_log, sizeof(g_last_log), "Completed %s wipe on %s (%s mode)", g_op.method, g_op.drive, g_op.mode);
    }

    progress = g_op.logicalBytesToProcess > 0ULL ? (int)((g_op.processedBytes * 100ULL) / g_op.logicalBytesToProcess) : 0;
    if (progress > 100)
    {
        progress = 100;
    }
    stage = resolve_stage();

    snprintf(out, sizeof(out), "{\"ok\":true,\"opId\":\"%s\",\"drive\":\"%s\",\"method\":\"%s\",\"mode\":\"%s\",\"progress\":%d,\"processedBytes\":%llu,\"totalBytes\":%llu,\"active\":%s,\"done\":%s,\"cancelled\":%s,\"stage\":\"%s\",\"ioError\":%s,\"log\":\"%s\"}", g_op.opId, g_op.drive, g_op.method, g_op.mode, progress, g_op.processedBytes, g_op.logicalBytesToProcess, g_op.active ? "true" : "false", g_op.done ? "true" : "false", g_op.cancelled ? "true" : "false", stage, g_op.ioError ? "true" : "false", g_last_log);
    return out;
}

const char *securewipe_get_certificate_json(const char *opId)
{
    const char *safeOpId = opId ? opId : "";
    refresh_progress();
    if (g_op.opId[0] == '\0' || strcmp(safeOpId, g_op.opId) != 0)
    {
        snprintf(g_last_certificate, sizeof(g_last_certificate), "{\"ok\":false,\"code\":\"NOT_FOUND\",\"message\":\"Operation not found\"}");
        return g_last_certificate;
    }
    if (!g_op.done)
    {
        snprintf(g_last_certificate, sizeof(g_last_certificate), "{\"ok\":false,\"code\":\"NOT_FINISHED\",\"message\":\"Operation still in progress\"}");
        return g_last_certificate;
    }
    snprintf(g_last_certificate, sizeof(g_last_certificate), "{\"ok\":true,\"certificate\":{\"opId\":\"%s\",\"drive\":\"%s\",\"method\":\"%s\",\"mode\":\"%s\",\"result\":\"%s\",\"startedAtMs\":%llu,\"finishedAtMs\":%llu,\"processedBytes\":%llu,\"totalBytes\":%llu}}", g_op.opId, g_op.drive, g_op.method, g_op.mode, g_op.cancelled ? "cancelled" : "completed", g_op.startedAtMs, g_op.finishedAtMs, g_op.processedBytes, g_op.logicalBytesToProcess);
    return g_last_certificate;
}
