#include "utils.h"

#include <mbedtls/base64.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEFAULT_CFG_PATH "~/.config/remotesc.json"
#define DEFAULT_ADDR "127.0.0.1:44555"

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

#define HOST_ULONG_LEN sizeof(CK_ULONG)
#define NET_ULONG_LEN 4
#define NET_UNAVAILABLE_INFORMATION 0xffffffff

bool __rsc_dbg = false;

static cJSON *read_file(const char *fname);

void debug(const char *func, int line, const char *fmt, ...)
{
    if (!__rsc_dbg) {
        return;
    }
    va_list args;
    fprintf(stderr, "DBG %s:%d ", func, line);
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fprintf(stderr, "\n");
}

void padded_copy(unsigned char *dst, const char *src, size_t dst_size)
{
    memset(dst, ' ', dst_size);
    size_t src_len = strlen(src);
    if (src_len > dst_size) {
        memcpy(dst, src, dst_size);
    } else {
        memcpy(dst, src, src_len);
    }
}

struct rsc_config *parse_config()
{
    struct rsc_config *cfg = NULL;
    const char *cfg_path = getenv("REMOTESC_CONFIG");
    if (cfg_path == NULL) {
        cfg_path = DEFAULT_CFG_PATH;
    }

    const char *addr = getenv("REMOTESC_ADDR");
    const char *fingerprint = getenv("REMOTESC_FINGERPRINT");
    const char *secret = getenv("REMOTESC_SECRET");

    cJSON *root = read_file(cfg_path);
    if (cJSON_IsObject(root)) {
        cJSON *obj;
        obj = cJSON_GetObjectItem(root, "addr");
        if (addr == NULL && cJSON_IsString(obj)) {
            addr = obj->valuestring;
        }
        obj = cJSON_GetObjectItem(root, "fingerprint");
        if (fingerprint == NULL && cJSON_IsString(obj)) {
            fingerprint = obj->valuestring;
        }
        obj = cJSON_GetObjectItem(root, "secret");
        if (secret == NULL && cJSON_IsString(obj)) {
            secret = obj->valuestring;
        }
    }

    if (addr == NULL) {
        addr = DEFAULT_ADDR;
    }
    if (fingerprint == NULL) {
        DBG("fingerprint not configured");
        goto out;
    }
    if (secret == NULL) {
        DBG("shared secret not configured");
        goto out;
    }

    cfg = malloc(sizeof(*cfg));
    cfg->addr = strdup(addr);
    cfg->fingerprint = strdup(fingerprint);
    cfg->secret = strdup(secret);
out:
    cJSON_Delete(root);
    return cfg;
}

void free_config(struct rsc_config *cfg)
{
    if (cfg == NULL) {
        return;
    }
    free(cfg->addr);
    free(cfg->fingerprint);
    free(cfg->secret);
    free(cfg);
}

cJSON *wrapAttributeArr(CK_ATTRIBUTE_PTR attrs, CK_ULONG count, bool getter)
{
    char buf[MAX_CRYPTO_OBJ_SIZE];
    cJSON *objs = cJSON_CreateArray();
    size_t str_len;

    for (CK_ULONG i = 0; i < count; i++) {
        cJSON *tmp = cJSON_CreateObject();
        cJSON_AddItemToArray(objs, tmp);

        cJSON_AddNumberToObject(tmp, "type", attrs[i].type);
        // request length
        if (attrs[i].pValue == NULL) {
            continue;
        }

        CK_ULONG len = attrs[i].ulValueLen;
        switch (attrs[i].type) {
        case CKA_CLASS:
        case CKA_CERTIFICATE_TYPE:
        case CKA_KEY_TYPE:
        case CKA_MODULUS_BITS:
            len = MIN(len, NET_ULONG_LEN);
        }

        if (!getter) {
            if (mbedtls_base64_encode((unsigned char *)buf, sizeof(buf), &str_len, attrs[i].pValue, len) != 0) {
                DBG("buffer too small");
                cJSON_Delete(objs);
                return NULL;
            }
            buf[str_len] = '\0';
            cJSON_AddStringToObject(tmp, "value", buf);
        }
        cJSON_AddNumberToObject(tmp, "valueLen", len);
    }
    return objs;
}

bool unwrapAttributeArr(cJSON *objs, CK_ATTRIBUTE_PTR attrs, CK_ULONG count)
{
    unsigned char buf[MAX_CRYPTO_OBJ_SIZE];
    int sz = cJSON_GetArraySize(objs);
    CK_ULONG len, dummy;

    for (int i = 0; i < sz; i++) {
        if (i >= (int)count) {
            return false;
        }
        cJSON *tmp = cJSON_GetArrayItem(objs, i);

        FILL_INT_BY_JSON(tmp, "type", attrs[i].type);
        FILL_INT_BY_JSON(tmp, "valueLen", len);

        if (len == 0) {
            continue;
        }

        if (len == NET_UNAVAILABLE_INFORMATION) {
            attrs[i].ulValueLen = CK_UNAVAILABLE_INFORMATION;
            continue;
        }

        switch (attrs[i].type) {
        case CKA_CLASS:
        case CKA_CERTIFICATE_TYPE:
        case CKA_KEY_TYPE:
        case CKA_MODULUS_BITS:
            len = MAX(len, HOST_ULONG_LEN);
        }

        cJSON *val = cJSON_GetObjectItem(tmp, "value");
        if (!cJSON_IsString(val)) {
            // return length
            attrs[i].ulValueLen = len;
            continue;
        }

        if (len > attrs[i].ulValueLen) {
            return false;
        }

        memset(buf, 0, len);
        if (mbedtls_base64_decode(buf, sizeof(buf), &dummy, (unsigned char *)val->valuestring, strlen(val->valuestring)) != 0) {
            DBG("buffer too small");
            return false;
        }
        memcpy(attrs[i].pValue, buf, len);

        attrs[i].ulValueLen = len;
    }
    return true;
}

static cJSON *read_file(const char *fname)
{
    FILE *file = fopen(fname, "rb");
    if (file == NULL) {
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    size_t length = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *content = malloc(length);
    size_t n = fread(content, sizeof(char), length, file);
    fclose(file);

    cJSON *json = cJSON_ParseWithLength(content, n);
    free(content);
    return json;
}
