#pragma once

#include <pkcs11.h>

typedef struct rsc_ctx rsc_ctx;

rsc_ctx *rsc_open(const char *module, char **err);
void rsc_close(rsc_ctx *ctx);

CK_RV rsc_Initialize(rsc_ctx *ctx);
CK_RV rsc_Finalize(rsc_ctx *ctx);
CK_RV rsc_GetInfo(rsc_ctx *ctx, CK_INFO_PTR pInfo);
CK_RV rsc_GetSlotList(rsc_ctx *ctx, CK_BBOOL tokenPresent, CK_SLOT_ID_PTR pSlotList, CK_ULONG_PTR pulCount);
CK_RV rsc_GetSlotInfo(rsc_ctx *ctx, CK_SLOT_ID slotID, CK_SLOT_INFO_PTR pInfo);
CK_RV rsc_GetTokenInfo(rsc_ctx *ctx, CK_SLOT_ID slotID, CK_TOKEN_INFO_PTR pInfo);
CK_RV rsc_OpenSession(rsc_ctx *ctx, CK_SLOT_ID slotID, CK_FLAGS flags, CK_SESSION_HANDLE_PTR phSession);
CK_RV rsc_CloseSession(rsc_ctx *ctx, CK_SESSION_HANDLE hSession);
CK_RV rsc_CloseAllSessions(rsc_ctx *ctx, CK_SLOT_ID slotID);
CK_RV rsc_Login(rsc_ctx *ctx, CK_SESSION_HANDLE hSession, CK_USER_TYPE userType, char *pPin, CK_ULONG ulPinLen);
CK_RV rsc_Logout(rsc_ctx *ctx, CK_SESSION_HANDLE hSession);
CK_RV rsc_GetAttributeValue(rsc_ctx *ctx, CK_SESSION_HANDLE hSession, CK_OBJECT_HANDLE hObject, CK_ATTRIBUTE_PTR pTemplate, CK_ULONG ulCount);
CK_RV rsc_FindObjectsInit(rsc_ctx *ctx, CK_SESSION_HANDLE hSession, CK_ATTRIBUTE_PTR pTemplate, CK_ULONG ulCount);
CK_RV rsc_FindObjects(rsc_ctx *ctx, CK_SESSION_HANDLE hSession, CK_OBJECT_HANDLE_PTR phObject, CK_ULONG ulMaxObjectCount, CK_ULONG_PTR pulObjectCount);
CK_RV rsc_FindObjectsFinal(rsc_ctx *ctx, CK_SESSION_HANDLE hSession);
CK_RV rsc_SignInit(rsc_ctx *ctx, CK_SESSION_HANDLE hSession, CK_MECHANISM_PTR pMechanism, CK_OBJECT_HANDLE hKey);
CK_RV rsc_Sign(rsc_ctx *ctx, CK_SESSION_HANDLE hSession, CK_BYTE_PTR pData, CK_ULONG ulDataLen, CK_BYTE_PTR pSignature, CK_ULONG_PTR pulSignatureLen);
CK_RV rsc_SignUpdate(rsc_ctx *ctx, CK_SESSION_HANDLE hSession, CK_BYTE_PTR pPart, CK_ULONG ulPartLen);
CK_RV rsc_SignFinal(rsc_ctx *ctx, CK_SESSION_HANDLE hSession, CK_BYTE_PTR pSignature, CK_ULONG_PTR pulSignatureLen);

static inline CK_FLAGS rsc_get_info_flags(CK_INFO_PTR info)
{
    return info->flags;
}

static inline CK_VOID_PTR rsc_get_attribute_value(CK_ATTRIBUTE_PTR attr)
{
    return attr->pValue;
}

static inline void rsc_set_attribute_value(CK_ATTRIBUTE_PTR attr, CK_VOID_PTR value)
{
    attr->pValue = value;
}
