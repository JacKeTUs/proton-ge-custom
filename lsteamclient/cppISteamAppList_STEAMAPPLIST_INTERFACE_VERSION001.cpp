/* This file is auto-generated, do not edit. */
#include "unix_private.h"

#if 0
#pragma makedep unix
#endif

NTSTATUS ISteamAppList_STEAMAPPLIST_INTERFACE_VERSION001_GetNumInstalledApps( void *args )
{
    struct ISteamAppList_STEAMAPPLIST_INTERFACE_VERSION001_GetNumInstalledApps_params *params = (struct ISteamAppList_STEAMAPPLIST_INTERFACE_VERSION001_GetNumInstalledApps_params *)args;
    struct u_ISteamAppList_STEAMAPPLIST_INTERFACE_VERSION001 *iface = (struct u_ISteamAppList_STEAMAPPLIST_INTERFACE_VERSION001 *)params->u_iface;
    params->_ret = iface->GetNumInstalledApps(  );
    return 0;
}

#if defined(__x86_64__) || defined(__aarch64__)
NTSTATUS wow64_ISteamAppList_STEAMAPPLIST_INTERFACE_VERSION001_GetNumInstalledApps( void *args )
{
    struct wow64_ISteamAppList_STEAMAPPLIST_INTERFACE_VERSION001_GetNumInstalledApps_params *params = (struct wow64_ISteamAppList_STEAMAPPLIST_INTERFACE_VERSION001_GetNumInstalledApps_params *)args;
    struct u_ISteamAppList_STEAMAPPLIST_INTERFACE_VERSION001 *iface = (struct u_ISteamAppList_STEAMAPPLIST_INTERFACE_VERSION001 *)params->u_iface;
    params->_ret = iface->GetNumInstalledApps(  );
    return 0;
}
#endif

NTSTATUS ISteamAppList_STEAMAPPLIST_INTERFACE_VERSION001_GetInstalledApps( void *args )
{
    struct ISteamAppList_STEAMAPPLIST_INTERFACE_VERSION001_GetInstalledApps_params *params = (struct ISteamAppList_STEAMAPPLIST_INTERFACE_VERSION001_GetInstalledApps_params *)args;
    struct u_ISteamAppList_STEAMAPPLIST_INTERFACE_VERSION001 *iface = (struct u_ISteamAppList_STEAMAPPLIST_INTERFACE_VERSION001 *)params->u_iface;
    params->_ret = iface->GetInstalledApps( params->pvecAppID, params->unMaxAppIDs );
    return 0;
}

#if defined(__x86_64__) || defined(__aarch64__)
NTSTATUS wow64_ISteamAppList_STEAMAPPLIST_INTERFACE_VERSION001_GetInstalledApps( void *args )
{
    struct wow64_ISteamAppList_STEAMAPPLIST_INTERFACE_VERSION001_GetInstalledApps_params *params = (struct wow64_ISteamAppList_STEAMAPPLIST_INTERFACE_VERSION001_GetInstalledApps_params *)args;
    struct u_ISteamAppList_STEAMAPPLIST_INTERFACE_VERSION001 *iface = (struct u_ISteamAppList_STEAMAPPLIST_INTERFACE_VERSION001 *)params->u_iface;
    params->_ret = iface->GetInstalledApps( params->pvecAppID, params->unMaxAppIDs );
    return 0;
}
#endif

NTSTATUS ISteamAppList_STEAMAPPLIST_INTERFACE_VERSION001_GetAppName( void *args )
{
    struct ISteamAppList_STEAMAPPLIST_INTERFACE_VERSION001_GetAppName_params *params = (struct ISteamAppList_STEAMAPPLIST_INTERFACE_VERSION001_GetAppName_params *)args;
    struct u_ISteamAppList_STEAMAPPLIST_INTERFACE_VERSION001 *iface = (struct u_ISteamAppList_STEAMAPPLIST_INTERFACE_VERSION001 *)params->u_iface;
    params->_ret = iface->GetAppName( params->nAppID, params->pchName, params->cchNameMax );
    return 0;
}

#if defined(__x86_64__) || defined(__aarch64__)
NTSTATUS wow64_ISteamAppList_STEAMAPPLIST_INTERFACE_VERSION001_GetAppName( void *args )
{
    struct wow64_ISteamAppList_STEAMAPPLIST_INTERFACE_VERSION001_GetAppName_params *params = (struct wow64_ISteamAppList_STEAMAPPLIST_INTERFACE_VERSION001_GetAppName_params *)args;
    struct u_ISteamAppList_STEAMAPPLIST_INTERFACE_VERSION001 *iface = (struct u_ISteamAppList_STEAMAPPLIST_INTERFACE_VERSION001 *)params->u_iface;
    params->_ret = iface->GetAppName( params->nAppID, params->pchName, params->cchNameMax );
    return 0;
}
#endif

NTSTATUS ISteamAppList_STEAMAPPLIST_INTERFACE_VERSION001_GetAppInstallDir( void *args )
{
    struct ISteamAppList_STEAMAPPLIST_INTERFACE_VERSION001_GetAppInstallDir_params *params = (struct ISteamAppList_STEAMAPPLIST_INTERFACE_VERSION001_GetAppInstallDir_params *)args;
    struct u_ISteamAppList_STEAMAPPLIST_INTERFACE_VERSION001 *iface = (struct u_ISteamAppList_STEAMAPPLIST_INTERFACE_VERSION001 *)params->u_iface;
    params->_ret = iface->GetAppInstallDir( params->nAppID, params->pchDirectory, params->cchNameMax );
    params->_ret = steamclient_unix_path_to_dos_path( params->_ret, params->pchDirectory, params->pchDirectory, params->cchNameMax, 0 );
    return 0;
}

#if defined(__x86_64__) || defined(__aarch64__)
NTSTATUS wow64_ISteamAppList_STEAMAPPLIST_INTERFACE_VERSION001_GetAppInstallDir( void *args )
{
    struct wow64_ISteamAppList_STEAMAPPLIST_INTERFACE_VERSION001_GetAppInstallDir_params *params = (struct wow64_ISteamAppList_STEAMAPPLIST_INTERFACE_VERSION001_GetAppInstallDir_params *)args;
    struct u_ISteamAppList_STEAMAPPLIST_INTERFACE_VERSION001 *iface = (struct u_ISteamAppList_STEAMAPPLIST_INTERFACE_VERSION001 *)params->u_iface;
    params->_ret = iface->GetAppInstallDir( params->nAppID, params->pchDirectory, params->cchNameMax );
    params->_ret = steamclient_unix_path_to_dos_path( params->_ret, params->pchDirectory, params->pchDirectory, params->cchNameMax, 0 );
    return 0;
}
#endif

NTSTATUS ISteamAppList_STEAMAPPLIST_INTERFACE_VERSION001_GetAppBuildId( void *args )
{
    struct ISteamAppList_STEAMAPPLIST_INTERFACE_VERSION001_GetAppBuildId_params *params = (struct ISteamAppList_STEAMAPPLIST_INTERFACE_VERSION001_GetAppBuildId_params *)args;
    struct u_ISteamAppList_STEAMAPPLIST_INTERFACE_VERSION001 *iface = (struct u_ISteamAppList_STEAMAPPLIST_INTERFACE_VERSION001 *)params->u_iface;
    params->_ret = iface->GetAppBuildId( params->nAppID );
    return 0;
}

#if defined(__x86_64__) || defined(__aarch64__)
NTSTATUS wow64_ISteamAppList_STEAMAPPLIST_INTERFACE_VERSION001_GetAppBuildId( void *args )
{
    struct wow64_ISteamAppList_STEAMAPPLIST_INTERFACE_VERSION001_GetAppBuildId_params *params = (struct wow64_ISteamAppList_STEAMAPPLIST_INTERFACE_VERSION001_GetAppBuildId_params *)args;
    struct u_ISteamAppList_STEAMAPPLIST_INTERFACE_VERSION001 *iface = (struct u_ISteamAppList_STEAMAPPLIST_INTERFACE_VERSION001 *)params->u_iface;
    params->_ret = iface->GetAppBuildId( params->nAppID );
    return 0;
}
#endif

