#include "vrclient_private.h"
#include "winreg.h"

WINE_DEFAULT_DEBUG_CHANNEL(vrclient);

static BOOL wait_vr_key_ready( HKEY vr_key )
{
    DWORD type, value, size = sizeof(value), wait_result;
    LSTATUS status;
    HANDLE event;
    BOOL ret;

    if ((status = RegQueryValueExA(vr_key, "state", NULL, &type, (BYTE *)&value, &size)))
    {
        ERR("Could not query VR state, status %lx\n", status);
        return FALSE;
    }
    if (type != REG_DWORD)
    {
        ERR("Invalid VR state type: %lx\n", type);
        return FALSE;
    }
    if (value) return value == 1;

    event = CreateEventA(NULL, FALSE, FALSE, NULL);
    ret = FALSE;
    while (1)
    {
        if (RegNotifyChangeKeyValue(vr_key, FALSE, REG_NOTIFY_CHANGE_LAST_SET, event, TRUE))
        {
            ERR("Error registering VR key change notification");
            break;
        }
        size = sizeof(value);
        if ((status = RegQueryValueExA(vr_key, "state", NULL, &type, (BYTE *)&value, &size)))
        {
            ERR("Couild not query VR state, status %lx\n", status);
            break;
        }
        if (value)
        {
            ret = value == 1;
            break;
        }
        while ((wait_result = WaitForSingleObject(event, 1000)) == WAIT_TIMEOUT)
            WARN("Waiting for VR to because ready\n");
        if (wait_result != WAIT_OBJECT_0)
        {
            ERR("Wait for VR state change failed\n");
            break;
        }
    }
    CloseHandle(event);
    return ret;
}

VkResult fixup_get_output_device_pre( HMODULE winevulkan, uint32_t *texture_type, VkInstance *instance )
{
    /* OpenVR IVRSystem::GetOutputDevice requires a VkInstance if textureType is Vulkan,
     * so we create one here. */
    BOOL has_get_device_properties2, has_external_memory_caps;
    PFN_vkGetInstanceProcAddr p_vkGetInstanceProcAddr;
    PFN_vkCreateInstance p_vkCreateInstance;
    const char **instance_extension_list;
    DWORD type, len, extension_count = 0;
    char *instance_extensions, *pos;
    VkResult vk_result;
    LSTATUS status;
    HKEY vr_key;

    VkInstanceCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
    };

    if (*texture_type != TextureType_DirectX && *texture_type != TextureType_DirectX12 && *texture_type != TextureType_DXGISharedHandle)
        return VK_SUCCESS;

    p_vkGetInstanceProcAddr = (void *)GetProcAddress(winevulkan, "vkGetInstanceProcAddr");
    p_vkCreateInstance = (void *)p_vkGetInstanceProcAddr(NULL, "vkCreateInstance");

    if ((status = RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\Wine\\VR", 0, KEY_READ, &vr_key)))
    {
        ERR("Failed to open OpenVR registry key, status %lx\n", status);
        return VK_ERROR_INITIALIZATION_FAILED;
    }
    if (!wait_vr_key_ready(vr_key))
    {
        RegCloseKey(vr_key);
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    if ((status = RegQueryValueExA(vr_key, "openvr_vulkan_instance_extensions", NULL, &type, NULL, &len)))
    {
        ERR("Could not query openvr vulkan instance extensions, status %lx\n", status);
        RegCloseKey(vr_key);
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    instance_extensions = calloc(len + 1, 1);
    status = RegQueryValueExA(vr_key, "openvr_vulkan_instance_extensions", NULL, &type, (BYTE *)instance_extensions, &len);
    RegCloseKey(vr_key);
    if (status)
    {
        ERR("Could not query openvr vulkan instance extensions, status %lx\n", status);
        free(instance_extensions);
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    TRACE("Creating VkInstance for IVRSystem::GetOutputDevice\n");

    has_get_device_properties2 = strstr(instance_extensions, VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME) != NULL;
    if (!has_get_device_properties2) extension_count += 1;
    has_external_memory_caps = strstr(instance_extensions, VK_KHR_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME) != NULL;
    if (!has_external_memory_caps) extension_count += 1;
    for (pos = instance_extensions; *pos == ' '; pos++);
    while (*pos != '\0')
    {
        extension_count++;
        for (; *pos != ' ' && *pos != '\0'; pos++);
        for (; *pos == ' '; pos++);
    }
    instance_extension_list = calloc(extension_count, sizeof(char *));

    extension_count = 0;
    if (!has_get_device_properties2)
        instance_extension_list[extension_count++] = VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME;
    if (!has_external_memory_caps)
        instance_extension_list[extension_count++] = VK_KHR_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME;
    for (pos = instance_extensions; *pos == ' '; pos++);
    while (TRUE)
    {
        instance_extension_list[extension_count++] = pos;
        for (; *pos != ' ' && *pos != '\0'; pos++);
        if (*pos == '\0') break;
        *(pos++) = '\0';
        for (; *pos == ' '; pos++);
    }
    create_info.ppEnabledExtensionNames = instance_extension_list;
    create_info.enabledExtensionCount = extension_count;

    vk_result = p_vkCreateInstance(&create_info, NULL, instance);
    free(instance_extension_list);
    free(instance_extensions);
    if (vk_result == VK_SUCCESS) *texture_type = TextureType_Vulkan;
    else ERR("Failed to create VkInstance\n");

    return vk_result;
}

void fixup_get_output_device_post( HMODULE winevulkan, uint64_t *device, VkInstance *instance, uint32_t original_texture_type )
{
    PFN_vkGetPhysicalDeviceProperties2 p_vkGetPhysicalDeviceProperties2;
    PFN_vkGetInstanceProcAddr p_vkGetInstanceProcAddr;
    PFN_vkDestroyInstance p_vkDestroyInstance;
    VkPhysicalDeviceIDProperties id_props = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ID_PROPERTIES,
    };
    VkPhysicalDeviceProperties2 props = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2,
        .pNext = &id_props,
    };

    if (
        original_texture_type != TextureType_DirectX &&
        original_texture_type != TextureType_DirectX12 &&
        original_texture_type != TextureType_DXGISharedHandle
    )
        return;

    p_vkGetInstanceProcAddr = (void *)GetProcAddress(winevulkan, "vkGetInstanceProcAddr");
    p_vkGetPhysicalDeviceProperties2 = (void *)p_vkGetInstanceProcAddr(*instance, "vkGetPhysicalDeviceProperties2");
    p_vkDestroyInstance = (void *)p_vkGetInstanceProcAddr(*instance, "vkDestroyInstance");

    if (*device != 0)
    {
        p_vkGetPhysicalDeviceProperties2((VkPhysicalDevice)(intptr_t)*device, &props);
        if (!id_props.deviceLUIDValid)
        {
            *device = 0;
            WARN("VkPhysicalDevice doesn't have valid LUID\n");
        }
        else
        {
            memcpy(device, id_props.deviceLUID, sizeof(uint8_t[8]));
            TRACE("LUID for OpenVR VkPhysicalDevice is %#I64x\n", *device);
        }
    }
    else
        ERR("Failed to get output device from OpenVR runtime\n");

    p_vkDestroyInstance(*instance, NULL);
    *instance = VK_NULL_HANDLE;
}


void __thiscall winIVRSystem_IVRSystem_003_GetDXGIOutputInfo( struct w_iface *_this, int32_t *pnAdapterIndex,
                                                              int32_t *pnAdapterOutputIndex )
{
    FIXME( "%p\n", _this );
    *pnAdapterIndex = 0;
    *pnAdapterOutputIndex = 0;
}

void __thiscall winIVRSystem_IVRSystem_004_GetDXGIOutputInfo( struct w_iface *_this, int32_t *pnAdapterIndex,
                                                              int32_t *pnAdapterOutputIndex )
{
    FIXME( "%p\n", _this );
    *pnAdapterIndex = 0;
    *pnAdapterOutputIndex = 0;
}

void __thiscall winIVRSystem_IVRSystem_005_GetDXGIOutputInfo( struct w_iface *_this, int32_t *pnAdapterIndex,
                                                              int32_t *pnAdapterOutputIndex )
{
    FIXME( "%p\n", _this );
    *pnAdapterIndex = 0;
    *pnAdapterOutputIndex = 0;
}

void __thiscall winIVRSystem_IVRSystem_006_GetDXGIOutputInfo( struct w_iface *_this, int32_t *pnAdapterIndex,
                                                              int32_t *pnAdapterOutputIndex )
{
    FIXME( "%p\n", _this );
    *pnAdapterIndex = 0;
    *pnAdapterOutputIndex = 0;
}

void __thiscall winIVRSystem_IVRSystem_009_GetDXGIOutputInfo( struct w_iface *_this, int32_t *pnAdapterIndex )
{
    FIXME( "%p\n", _this );
    *pnAdapterIndex = 0;
}

void __thiscall winIVRSystem_IVRSystem_010_GetDXGIOutputInfo( struct w_iface *_this, int32_t *pnAdapterIndex )
{
    FIXME( "%p\n", _this );
    *pnAdapterIndex = 0;
}

void __thiscall winIVRSystem_IVRSystem_011_GetDXGIOutputInfo( struct w_iface *_this, int32_t *pnAdapterIndex )
{
    FIXME( "%p\n", _this );
    *pnAdapterIndex = 0;
}

void __thiscall winIVRSystem_IVRSystem_012_GetDXGIOutputInfo( struct w_iface *_this, int32_t *pnAdapterIndex )
{
    FIXME( "%p\n", _this );
    *pnAdapterIndex = 0;
}

void __thiscall winIVRSystem_IVRSystem_014_GetDXGIOutputInfo( struct w_iface *_this, int32_t *pnAdapterIndex )
{
    FIXME( "%p\n", _this );
    *pnAdapterIndex = 0;
}

void __thiscall winIVRSystem_IVRSystem_015_GetDXGIOutputInfo( struct w_iface *_this, int32_t *pnAdapterIndex )
{
    FIXME( "%p\n", _this );
    *pnAdapterIndex = 0;
}

void __thiscall winIVRSystem_IVRSystem_016_GetDXGIOutputInfo( struct w_iface *_this, int32_t *pnAdapterIndex )
{
    FIXME( "%p\n", _this );
    *pnAdapterIndex = 0;
}

void __thiscall winIVRSystem_IVRSystem_017_GetDXGIOutputInfo( struct w_iface *_this, int32_t *pnAdapterIndex )
{
    FIXME( "%p\n", _this );
    *pnAdapterIndex = 0;
}

void __thiscall winIVRSystem_IVRSystem_019_GetDXGIOutputInfo( struct w_iface *_this, int32_t *pnAdapterIndex )
{
    FIXME( "%p\n", _this );
    *pnAdapterIndex = 0;
}

void __thiscall winIVRSystem_IVRSystem_020_GetDXGIOutputInfo( struct w_iface *_this, int32_t *pnAdapterIndex )
{
    FIXME( "%p\n", _this );
    *pnAdapterIndex = 0;
}

void __thiscall winIVRSystem_IVRSystem_021_GetDXGIOutputInfo( struct w_iface *_this, int32_t *pnAdapterIndex )
{
    FIXME( "%p\n", _this );
    *pnAdapterIndex = 0;
}

void __thiscall winIVRSystem_IVRSystem_022_GetDXGIOutputInfo( struct w_iface *_this, int32_t *pnAdapterIndex )
{
    FIXME( "%p\n", _this );
    *pnAdapterIndex = 0;
}

static const WCHAR winevulkanW[] = {'w','i','n','e','v','u','l','k','a','n','.','d','l','l',0};

void __thiscall winIVRSystem_IVRSystem_017_GetOutputDevice( struct w_iface *_this, uint64_t *pnDevice, uint32_t textureType, VkInstance_T *pInstance )
{
    struct IVRSystem_IVRSystem_017_GetOutputDevice_params params =
    {
        .u_iface = _this->u_iface,
        .pnDevice = pnDevice,
        .textureType = textureType,
        .pInstance = pInstance,
    };
    HMODULE winevulkan = LoadLibraryW(winevulkanW);

    TRACE("%p\n", _this);
    if (fixup_get_output_device_pre( winevulkan, &params.textureType, &params.pInstance ) != VK_SUCCESS)
        *pnDevice = 0;
    else
    {
        VRCLIENT_CALL( IVRSystem_IVRSystem_017_GetOutputDevice, &params );
        fixup_get_output_device_post( winevulkan, pnDevice, &params.pInstance, textureType );
    }

    FreeLibrary(winevulkan);
}

void __thiscall winIVRSystem_IVRSystem_019_GetOutputDevice( struct w_iface *_this, uint64_t *pnDevice, uint32_t textureType, VkInstance_T *pInstance )
{
    struct IVRSystem_IVRSystem_019_GetOutputDevice_params params =
    {
        .u_iface = _this->u_iface,
        .pnDevice = pnDevice,
        .textureType = textureType,
        .pInstance = pInstance,
    };
    HMODULE winevulkan = LoadLibraryW(winevulkanW);

    TRACE("%p\n", _this);
    if (fixup_get_output_device_pre( winevulkan, &params.textureType, &params.pInstance ) != VK_SUCCESS)
        *pnDevice = 0;
    else
    {
        VRCLIENT_CALL( IVRSystem_IVRSystem_019_GetOutputDevice, &params );
        fixup_get_output_device_post( winevulkan, pnDevice, &params.pInstance, textureType );
    }

    FreeLibrary(winevulkan);
}

void __thiscall winIVRSystem_IVRSystem_020_GetOutputDevice( struct w_iface *_this, uint64_t *pnDevice, uint32_t textureType, VkInstance_T *pInstance )
{
    struct IVRSystem_IVRSystem_020_GetOutputDevice_params params =
    {
        .u_iface = _this->u_iface,
        .pnDevice = pnDevice,
        .textureType = textureType,
        .pInstance = pInstance,
    };
    HMODULE winevulkan = LoadLibraryW(winevulkanW);

    TRACE("%p\n", _this);
    if (fixup_get_output_device_pre( winevulkan, &params.textureType, &params.pInstance ) != VK_SUCCESS)
        *pnDevice = 0;
    else
    {
        VRCLIENT_CALL( IVRSystem_IVRSystem_020_GetOutputDevice, &params );
        fixup_get_output_device_post( winevulkan, pnDevice, &params.pInstance, textureType );
    }

    FreeLibrary(winevulkan);
}

void __thiscall winIVRSystem_IVRSystem_021_GetOutputDevice( struct w_iface *_this, uint64_t *pnDevice, uint32_t textureType, VkInstance_T *pInstance )
{
    struct IVRSystem_IVRSystem_021_GetOutputDevice_params params =
    {
        .u_iface = _this->u_iface,
        .pnDevice = pnDevice,
        .textureType = textureType,
        .pInstance = pInstance,
    };
    HMODULE winevulkan = LoadLibraryW(winevulkanW);

    TRACE("%p\n", _this);
    if (fixup_get_output_device_pre( winevulkan, &params.textureType, &params.pInstance ) != VK_SUCCESS)
        *pnDevice = 0;
    else
    {
        VRCLIENT_CALL( IVRSystem_IVRSystem_021_GetOutputDevice, &params );
        fixup_get_output_device_post( winevulkan, pnDevice, &params.pInstance, textureType );
    }

    FreeLibrary(winevulkan);
}


void __thiscall winIVRSystem_IVRSystem_022_GetOutputDevice( struct w_iface *_this, uint64_t *pnDevice, uint32_t textureType, VkInstance_T *pInstance )
{
    struct IVRSystem_IVRSystem_022_GetOutputDevice_params params =
    {
        .u_iface = _this->u_iface,
        .pnDevice = pnDevice,
        .textureType = textureType,
        .pInstance = pInstance,
    };
    HMODULE winevulkan = LoadLibraryW(winevulkanW);

    TRACE("%p\n", _this);
    if (fixup_get_output_device_pre( winevulkan, &params.textureType, &params.pInstance ) != VK_SUCCESS)
        *pnDevice = 0;
    else
    {
        VRCLIENT_CALL( IVRSystem_IVRSystem_022_GetOutputDevice, &params );
        fixup_get_output_device_post( winevulkan, pnDevice, &params.pInstance, textureType );
    }

    FreeLibrary(winevulkan);
}

