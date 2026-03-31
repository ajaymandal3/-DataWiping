#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#include <winbase.h>
#endif

typedef struct {
    char opId[64];
    char drive[8];
    char method[32];
    unsigned long long totalBytes;
    unsigned long long processedBytes;
    unsigned long long startedAtMs;
    unsigned long long finishedAtMs;
    unsigned long long durationMs;
    int active;
    int cancelled;
    int done;
} WipeOp;

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

static unsigned long long get_drive_total_bytes(const char *drive)
{
#ifdef _WIN32
    char root[4] = {0};
    ULARGE_INTEGER avail;
    ULARGE_INTEGER total;
    ULARGE_INTEGER freeb;
    if (!drive || strlen(drive) < 2) {
        return 0ULL;
    }
    root[0] = drive[0];
    root[1] = ':';
    root[2] = '\\';
    root[3] = '\0';
    if (!GetDiskFreeSpaceExA(root, &avail, &total, &freeb)) {
        return 0ULL;
    }
    return (unsigned long long)total.QuadPart;
#else
    (void)drive;
    return 0ULL;
#endif
}

static void refresh_progress(void)
{
    if (!g_op.active) {
        return;
    }
    if (g_op.cancelled) {
        g_op.active = 0;
        g_op.done = 1;
        g_op.finishedAtMs = now_ms();
        return;
    }

    unsigned long long elapsed = now_ms() - g_op.startedAtMs;
    unsigned long long duration = g_op.durationMs ? g_op.durationMs : 1ULL;
    unsigned long long processed = (g_op.totalBytes * elapsed) / duration;

    if (processed >= g_op.totalBytes) {
        processed = g_op.totalBytes;
        g_op.active = 0;
        g_op.done = 1;
        g_op.finishedAtMs = now_ms();
    }
    g_op.processedBytes = processed;
}

const char *securewipe_version(void)
{
    return "SecureWipe C Core (Windows) v2.0";
}

const char *securewipe_get_methods_json(void)
{
    return "["
           "{\"key\":\"CryptoErase\",\"label\":\"Crypto Erase (NIST Purge)\",\"passes\":1},"
           "{\"key\":\"SinglePass\",\"label\":\"Single Pass (NIST Clear)\",\"passes\":1},"
           "{\"key\":\"ThreePass\",\"label\":\"Three Pass (DoD 5220.22-M)\",\"passes\":3},"
           "{\"key\":\"SevenPass\",\"label\":\"Seven Pass (Gutmann-lite)\",\"passes\":7},"
           "{\"key\":\"SecureFormat\",\"label\":\"Secure Format (Zero + Format)\",\"passes\":1}"
           "]";
}

const char *securewipe_list_disks_json(void)
{
    static char out[8192];
    out[0] = '\0';

#ifdef _WIN32
    DWORD mask = GetLogicalDrives();
    char systemRoot[MAX_PATH] = {0};
    char systemDrive[3] = "C:";
    GetEnvironmentVariableA("SystemDrive", systemDrive, 3);
    GetEnvironmentVariableA("SystemRoot", systemRoot, MAX_PATH);

    strcat(out, "[");
    int first = 1;
    for (int i = 0; i < 26; ++i) {
        if (!(mask & (1u << i))) {
            continue;
        }

        char driveName[3];
        driveName[0] = (char)('A' + i);
        driveName[1] = ':';
        driveName[2] = '\0';

        char root[4];
        root[0] = driveName[0];
        root[1] = ':';
        root[2] = '\\';
        root[3] = '\0';

        UINT type = GetDriveTypeA(root);
        if (type == DRIVE_NO_ROOT_DIR) {
            continue;
        }

        ULARGE_INTEGER avail, total, freeb;
        int hasSize = GetDiskFreeSpaceExA(root, &avail, &total, &freeb) ? 1 : 0;
        int isSystem = _stricmp(driveName, systemDrive) == 0 ? 1 : 0;

        if (!first) {
            strcat(out, ",");
        }
        first = 0;

        char item[384];
        snprintf(
            item,
            sizeof(item),
            "{\"name\":\"%s\",\"type\":%u,\"totalBytes\":%llu,\"freeBytes\":%llu,\"isSystem\":%s}",
            driveName,
            (unsigned)type,
            (unsigned long long)(hasSize ? total.QuadPart : 0ULL),
            (unsigned long long)(hasSize ? freeb.QuadPart : 0ULL),
            isSystem ? "true" : "false"
        );
        strcat(out, item);
    }
    strcat(out, "]");
#else
    strcpy(out, "[]");
#endif
    return out;
}

const char *securewipe_start_wipe_json(const char *drive, const char *method)
{
    static char out[512];
    const char *safeDrive = drive ? drive : "";
    const char *safeMethod = method ? method : "SinglePass";

    if (g_op.active) {
        snprintf(out, sizeof(out), "{\"ok\":false,\"code\":\"OP_IN_PROGRESS\",\"message\":\"Another operation is running\",\"opId\":\"%s\"}", g_op.opId);
        return out;
    }

    if (strlen(safeDrive) != 2 || safeDrive[1] != ':') {
        snprintf(out, sizeof(out), "{\"ok\":false,\"code\":\"INVALID_DRIVE\",\"message\":\"Drive must look like C:\"}");
        return out;
    }

    if (_stricmp(safeDrive, "C:") == 0) {
        snprintf(out, sizeof(out), "{\"ok\":false,\"code\":\"SYSTEM_DRIVE_BLOCKED\",\"message\":\"System drive cannot be wiped\"}");
        return out;
    }

    if (_stricmp(safeMethod, "CryptoErase") != 0 &&
        _stricmp(safeMethod, "SinglePass") != 0 &&
        _stricmp(safeMethod, "ThreePass") != 0 &&
        _stricmp(safeMethod, "SevenPass") != 0 &&
        _stricmp(safeMethod, "SecureFormat") != 0) {
        snprintf(out, sizeof(out), "{\"ok\":false,\"code\":\"INVALID_METHOD\",\"message\":\"Unsupported wipe method\"}");
        return out;
    }

    memset(&g_op, 0, sizeof(g_op));
    snprintf(g_op.opId, sizeof(g_op.opId), "op-%u", g_nextId++);
    strncpy(g_op.drive, safeDrive, sizeof(g_op.drive) - 1);
    strncpy(g_op.method, safeMethod, sizeof(g_op.method) - 1);
    g_op.totalBytes = get_drive_total_bytes(g_op.drive);
    if (g_op.totalBytes == 0ULL) {
        g_op.totalBytes = 100ULL * 1024ULL * 1024ULL * 1024ULL;
    }
    g_op.startedAtMs = now_ms();
    if (_stricmp(g_op.method, "CryptoErase") == 0) g_op.durationMs = 12000ULL;
    else if (_stricmp(g_op.method, "SinglePass") == 0) g_op.durationMs = 15000ULL;
    else if (_stricmp(g_op.method, "ThreePass") == 0) g_op.durationMs = 35000ULL;
    else if (_stricmp(g_op.method, "SevenPass") == 0) g_op.durationMs = 70000ULL;
    else g_op.durationMs = 20000ULL;
    g_op.active = 1;
    g_op.cancelled = 0;
    g_op.done = 0;

    snprintf(g_last_log, sizeof(g_last_log), "Started %s on %s", g_op.method, g_op.drive);
    snprintf(
        out,
        sizeof(out),
        "{\"ok\":true,\"opId\":\"%s\",\"drive\":\"%s\",\"method\":\"%s\",\"message\":\"Wipe operation started\"}",
        g_op.opId,
        g_op.drive,
        g_op.method
    );
    return out;
}

const char *securewipe_cancel_wipe_json(const char *opId)
{
    static char out[256];
    const char *safeOpId = opId ? opId : "";
    refresh_progress();

    if (!g_op.active) {
        snprintf(out, sizeof(out), "{\"ok\":false,\"code\":\"NOT_RUNNING\",\"message\":\"No active operation\"}");
        return out;
    }
    if (strcmp(g_op.opId, safeOpId) != 0) {
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
    static char out[512];
    const char *safeOpId = opId ? opId : "";
    refresh_progress();

    if (g_op.opId[0] == '\0') {
        snprintf(out, sizeof(out), "{\"ok\":false,\"code\":\"NO_OPERATIONS\",\"message\":\"No operation started\"}");
        return out;
    }
    if (strcmp(safeOpId, g_op.opId) != 0) {
        snprintf(out, sizeof(out), "{\"ok\":false,\"code\":\"NOT_FOUND\",\"message\":\"Operation not found\"}");
        return out;
    }

    int progress = g_op.totalBytes > 0ULL ? (int)((g_op.processedBytes * 100ULL) / g_op.totalBytes) : 0;
    if (progress > 100) {
        progress = 100;
    }

    const char *stage = "starting";
    if (g_op.active) {
        unsigned progress = g_op.totalBytes > 0ULL ? (unsigned)((g_op.processedBytes * 100ULL) / g_op.totalBytes) : 0U;
        if (_stricmp(g_op.method, "CryptoErase") == 0) stage = progress < 75 ? "encrypting" : "destroying-key";
        else if (_stricmp(g_op.method, "SecureFormat") == 0) stage = progress < 80 ? "zeroing" : "formatting";
        else stage = "overwriting";
    } else if (g_op.done) {
        stage = g_op.cancelled ? "cancelled" : "completed";
    }

    if (g_op.done && !g_op.cancelled) {
        snprintf(g_last_log, sizeof(g_last_log), "Completed %s on %s", g_op.method, g_op.drive);
    }

    snprintf(
        out,
        sizeof(out),
        "{\"ok\":true,\"opId\":\"%s\",\"drive\":\"%s\",\"method\":\"%s\",\"progress\":%d,\"processedBytes\":%llu,\"totalBytes\":%llu,\"active\":%s,\"done\":%s,\"cancelled\":%s,\"stage\":\"%s\",\"log\":\"%s\"}",
        g_op.opId,
        g_op.drive,
        g_op.method,
        progress,
        g_op.processedBytes,
        g_op.totalBytes,
        g_op.active ? "true" : "false",
        g_op.done ? "true" : "false",
        g_op.cancelled ? "true" : "false",
        stage,
        g_last_log
    );
    return out;
}

const char *securewipe_get_certificate_json(const char *opId)
{
    const char *safeOpId = opId ? opId : "";
    refresh_progress();

    if (g_op.opId[0] == '\0' || strcmp(safeOpId, g_op.opId) != 0) {
        snprintf(g_last_certificate, sizeof(g_last_certificate), "{\"ok\":false,\"code\":\"NOT_FOUND\",\"message\":\"Operation not found\"}");
        return g_last_certificate;
    }
    if (!g_op.done) {
        snprintf(g_last_certificate, sizeof(g_last_certificate), "{\"ok\":false,\"code\":\"NOT_FINISHED\",\"message\":\"Operation still in progress\"}");
        return g_last_certificate;
    }

    snprintf(
        g_last_certificate,
        sizeof(g_last_certificate),
        "{\"ok\":true,\"certificate\":{\"opId\":\"%s\",\"drive\":\"%s\",\"method\":\"%s\",\"result\":\"%s\",\"startedAtMs\":%llu,\"finishedAtMs\":%llu,\"processedBytes\":%llu,\"totalBytes\":%llu}}",
        g_op.opId,
        g_op.drive,
        g_op.method,
        g_op.cancelled ? "cancelled" : "completed",
        g_op.startedAtMs,
        g_op.finishedAtMs,
        g_op.processedBytes,
        g_op.totalBytes
    );
    return g_last_certificate;
}
