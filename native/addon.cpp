#include <node_api.h>
#include <assert.h>

extern "C" {
const char *securewipe_version(void);
const char *securewipe_get_methods_json(void);
const char *securewipe_list_disks_json(void);
const char *securewipe_start_wipe_json(const char *drive, const char *method, const char *mode);
const char *securewipe_get_wipe_status_json(const char *opId);
const char *securewipe_cancel_wipe_json(const char *opId);
const char *securewipe_get_certificate_json(const char *opId);
}

static napi_value GetMethods(napi_env env, napi_callback_info info)
{
    (void)info;
    const char *json = securewipe_get_methods_json();
    napi_value out;
    napi_status status = napi_create_string_utf8(env, json, NAPI_AUTO_LENGTH, &out);
    assert(status == napi_ok);
    return out;
}

static napi_value GetVersion(napi_env env, napi_callback_info info)
{
    (void)info;
    const char *version = securewipe_version();
    napi_value out;
    napi_status status = napi_create_string_utf8(env, version, NAPI_AUTO_LENGTH, &out);
    assert(status == napi_ok);
    return out;
}

static napi_value GetDisks(napi_env env, napi_callback_info info)
{
    (void)info;
    const char *json = securewipe_list_disks_json();
    napi_value out;
    napi_status status = napi_create_string_utf8(env, json, NAPI_AUTO_LENGTH, &out);
    assert(status == napi_ok);
    return out;
}

static napi_value StartWipe(napi_env env, napi_callback_info info)
{
    size_t argc = 3;
    napi_value argv[3];
    napi_status status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    assert(status == napi_ok);

    char drive[64] = {0};
    char method[64] = "SinglePass";
    char mode[16] = "real";
    if (argc >= 1) {
        size_t copied = 0;
        status = napi_get_value_string_utf8(env, argv[0], drive, sizeof(drive), &copied);
        assert(status == napi_ok);
        (void)copied;
    }
    if (argc >= 2) {
        size_t copied = 0;
        status = napi_get_value_string_utf8(env, argv[1], method, sizeof(method), &copied);
        assert(status == napi_ok);
        (void)copied;
    }
    if (argc >= 3) {
        size_t copied = 0;
        status = napi_get_value_string_utf8(env, argv[2], mode, sizeof(mode), &copied);
        assert(status == napi_ok);
        (void)copied;
    }

    const char *json = securewipe_start_wipe_json(drive, method, mode);
    napi_value out;
    status = napi_create_string_utf8(env, json, NAPI_AUTO_LENGTH, &out);
    assert(status == napi_ok);
    return out;
}

static napi_value GetWipeStatus(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1];
    napi_status status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    assert(status == napi_ok);

    char opId[64] = {0};
    if (argc >= 1) {
        size_t copied = 0;
        status = napi_get_value_string_utf8(env, argv[0], opId, sizeof(opId), &copied);
        assert(status == napi_ok);
        (void)copied;
    }

    const char *json = securewipe_get_wipe_status_json(opId);
    napi_value out;
    status = napi_create_string_utf8(env, json, NAPI_AUTO_LENGTH, &out);
    assert(status == napi_ok);
    return out;
}

static napi_value CancelWipe(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1];
    napi_status status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    assert(status == napi_ok);

    char opId[64] = {0};
    if (argc >= 1) {
        size_t copied = 0;
        status = napi_get_value_string_utf8(env, argv[0], opId, sizeof(opId), &copied);
        assert(status == napi_ok);
        (void)copied;
    }

    const char *json = securewipe_cancel_wipe_json(opId);
    napi_value out;
    status = napi_create_string_utf8(env, json, NAPI_AUTO_LENGTH, &out);
    assert(status == napi_ok);
    return out;
}

static napi_value GetCertificate(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1];
    napi_status status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    assert(status == napi_ok);

    char opId[64] = {0};
    if (argc >= 1) {
        size_t copied = 0;
        status = napi_get_value_string_utf8(env, argv[0], opId, sizeof(opId), &copied);
        assert(status == napi_ok);
        (void)copied;
    }

    const char *json = securewipe_get_certificate_json(opId);
    napi_value out;
    status = napi_create_string_utf8(env, json, NAPI_AUTO_LENGTH, &out);
    assert(status == napi_ok);
    return out;
}

static napi_value Init(napi_env env, napi_value exports)
{
    napi_value fn;
    napi_status status = napi_create_function(env, "getVersion", NAPI_AUTO_LENGTH, GetVersion, nullptr, &fn);
    assert(status == napi_ok);

    status = napi_set_named_property(env, exports, "getVersion", fn);
    assert(status == napi_ok);
    napi_value methodsFn;
    status = napi_create_function(env, "getMethods", NAPI_AUTO_LENGTH, GetMethods, nullptr, &methodsFn);
    assert(status == napi_ok);
    status = napi_set_named_property(env, exports, "getMethods", methodsFn);
    assert(status == napi_ok);

    napi_value disksFn;
    status = napi_create_function(env, "getDisks", NAPI_AUTO_LENGTH, GetDisks, nullptr, &disksFn);
    assert(status == napi_ok);

    status = napi_set_named_property(env, exports, "getDisks", disksFn);
    assert(status == napi_ok);

    napi_value startWipeFn;
    status = napi_create_function(env, "startWipe", NAPI_AUTO_LENGTH, StartWipe, nullptr, &startWipeFn);
    assert(status == napi_ok);
    status = napi_set_named_property(env, exports, "startWipe", startWipeFn);
    assert(status == napi_ok);

    napi_value statusFn;
    status = napi_create_function(env, "getWipeStatus", NAPI_AUTO_LENGTH, GetWipeStatus, nullptr, &statusFn);
    assert(status == napi_ok);
    status = napi_set_named_property(env, exports, "getWipeStatus", statusFn);
    assert(status == napi_ok);

    napi_value cancelFn;
    status = napi_create_function(env, "cancelWipe", NAPI_AUTO_LENGTH, CancelWipe, nullptr, &cancelFn);
    assert(status == napi_ok);
    status = napi_set_named_property(env, exports, "cancelWipe", cancelFn);
    assert(status == napi_ok);

    napi_value certFn;
    status = napi_create_function(env, "getCertificate", NAPI_AUTO_LENGTH, GetCertificate, nullptr, &certFn);
    assert(status == napi_ok);
    status = napi_set_named_property(env, exports, "getCertificate", certFn);
    assert(status == napi_ok);
    return exports;
}

NAPI_MODULE(NODE_GYP_MODULE_NAME, Init)
