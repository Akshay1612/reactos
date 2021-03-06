/*
 * PROJECT:     NetAPI DLL
 * LICENSE:     GPL-2.0 (https://spdx.org/licenses/GPL-2.0)
 * PURPOSE:     Miscellaneous functions
 * COPYRIGHT:   Copyright 2017 Eric Kohl (eric.kohl@reactos.org)
 */

/* INCLUDES ******************************************************************/

#include "netapi32.h"

#include <rpc.h>
#include "srvsvc_c.h"
#include "wkssvc_c.h"


WINE_DEFAULT_DEBUG_CHANNEL(netapi32);

/* FUNCTIONS *****************************************************************/

NET_API_STATUS
WINAPI
NetRegisterDomainNameChangeNotification(
    _Out_ PHANDLE NotificationEventHandle)
{
    HANDLE EventHandle;
    NTSTATUS Status;

    TRACE("NetRegisterDomainNameChangeNotification(%p)\n",
          NotificationEventHandle);

    if (NotificationEventHandle == NULL)
        return ERROR_INVALID_PARAMETER;

    EventHandle = CreateEventW(NULL, FALSE, FALSE, NULL);
    if (EventHandle == NULL)
        return GetLastError();

    Status = LsaRegisterPolicyChangeNotification(PolicyNotifyDnsDomainInformation,
                                                 NotificationEventHandle);
    if (!NT_SUCCESS(Status))
    {
        CloseHandle(EventHandle);
        return NetpNtStatusToApiStatus(Status);
    }

    *NotificationEventHandle = EventHandle;

    return NERR_Success;
}


NET_API_STATUS
WINAPI
NetStatisticsGet(
    _In_ LPWSTR server,
    _In_ LPWSTR service,
    _In_ DWORD level,
    _In_ DWORD options,
    _Out_ LPBYTE *bufptr)
{
    NET_API_STATUS status = ERROR_NOT_SUPPORTED;

    TRACE("NetStatisticsGet(%s %s %lu %lu %p)\n",
          debugstr_w(server), debugstr_w(service), level, options, bufptr);

    *bufptr = NULL;

    if (_wcsicmp(service, L"LanmanWorkstation") == 0)
    {
        if (level != 0)
            return ERROR_INVALID_LEVEL;

        if (options != 0)
            return ERROR_INVALID_PARAMETER;

        RpcTryExcept
        {
            status = NetrWorkstationStatisticsGet(server,
                                                  L"LanmanWorkstation",
                                                  level,
                                                  options,
                                                  (LPSTAT_WORKSTATION_0*)bufptr);
        }
        RpcExcept(EXCEPTION_EXECUTE_HANDLER)
        {
            status = I_RpcMapWin32Status(RpcExceptionCode());
        }
        RpcEndExcept;
    }
    else if (_wcsicmp(service, L"LanmanServer") == 0)
    {
        if (level != 0)
            return ERROR_INVALID_LEVEL;

        if (options != 0)
            return ERROR_INVALID_PARAMETER;

        RpcTryExcept
        {
            status = NetrServerStatisticsGet(server,
                                             L"LanmanServer",
                                             level,
                                             options,
                                             (LPSTAT_SERVER_0 *)bufptr);
        }
        RpcExcept(EXCEPTION_EXECUTE_HANDLER)
        {
            status = I_RpcMapWin32Status(RpcExceptionCode());
        }
        RpcEndExcept;
    }

    return status;
}


NET_API_STATUS
WINAPI
NetUnregisterDomainNameChangeNotification(
    _In_ HANDLE NotificationEventHandle)
{
    NTSTATUS Status;

    TRACE("NetUnregisterDomainNameChangeNotification(%p)\n",
          NotificationEventHandle);

    if (NotificationEventHandle == NULL)
        return ERROR_INVALID_PARAMETER;

    Status = LsaUnregisterPolicyChangeNotification(PolicyNotifyDnsDomainInformation,
                                                   NotificationEventHandle);

    return NetpNtStatusToApiStatus(Status);
}


NET_API_STATUS
WINAPI
NetpNtStatusToApiStatus(
    _In_ NTSTATUS Status)
{
    NET_API_STATUS ApiStatus;

    switch (Status)
    {
        case STATUS_SUCCESS:
            ApiStatus = NERR_Success;
            break;

        case STATUS_INVALID_ACCOUNT_NAME:
            ApiStatus = NERR_BadUsername;
            break;

        case STATUS_PASSWORD_RESTRICTION:
            ApiStatus = NERR_PasswordTooShort;
            break;

        default:
            ApiStatus = RtlNtStatusToDosError(Status);
            break;
    }

    return ApiStatus;
}

/* EOF */
