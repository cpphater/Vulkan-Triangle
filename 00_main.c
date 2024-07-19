#pragma warning(disable:  4201) // nameless struct/union
#pragma warning(disable:  4191) // (might be dangerous) type cast (FARPROC to PFN)
#pragma warning(disable:  5045) // specture mitigation

#include "01_commons.h"

#include "03_debug.c"
#include "03_assert.c"

#include "99_win32_errstr.c"
#include "99_create_window.c"

//
// MACROS:
//

#define VK_GET_PROC_ADDR(__NAME__, __VALUE__, __INSTANCE__) \
    literal_t _pfn_name_ = STRINGIFY(__NAME__);             \
    PFN_##__NAME__ __VALUE__ = (PFN_##__NAME__)             \
    vkGetInstanceProcAddr(__INSTANCE__, STRINGIFY(__NAME__))\

//
// TYPES/TYPEDEFS
//

typedef u8 keymap_t;

//           prev_state 
//            |      scan_code
//            . xxxx ........
// LPARAM  0b11111111111111110000000000000000;
//           . .    .        ................
//           |alt  ext           repeat
//       transition 

union win32_key {
    struct {
        LPARAM _repeat    : 16;
        LPARAM _scan_code : 8;
        LPARAM extended   : 1;
        LPARAM reserved   : 4;
        LPARAM context    : 1;
        LPARAM prev_state : 1;
        LPARAM transition : 1;
    };

    struct {
        s16 repeat;
        keymap_t scan_code;
        u8 _;
    };

    LPARAM whole;
};

typedef u32 index_t;
typedef index_t family_index_t;



enum vk_device_queue_index {
    QUEUE_INDEX_GRAPHICS,
    QUEUE_INDEX_PRESENT,

    QUEUE_INDEX_ENUM_MAX_SIZE,
};

union vk_device_queue_family_idx {
    struct  {
        family_index_t present;
        family_index_t graphics;
    };
};

//
// GLOBALS
//

s32 GLOBAL g_running = 0x1;

struct global_memory {
    struct {
        char  layer_names_buffer[BUFFER_SIZE_VK_LAYER_NAMES];
        char  EXT_names_buffer[BUFFER_SIZE_VK_EXT_NAMES];

        u32   EXT_count;
        u32   layer_count;

        char* EXT_names[VK_MAX_COUNT_EXT];
        char* layer_names[VK_MAX_COUNT_LAYERS];

        struct vk_stored_PDEV_familes {
            union vk_device_queue_family_idx  indecies;
            VkQueueFamilyProperties           property[VK_MAX_COUNT_QUEUE_FAMILY];
            u32                               count;
        }
        PDEV_familes;

        slop_t _[4];

        VkQueue graphics_queue;
        VkQueue present_queue;
    };

    union {
        struct {
            VkLayerProperties       available_layer[VK_MAX_COUNT_LAYERS];
            VkExtensionProperties   available_EXT[VK_MAX_COUNT_EXT];
            u32                     available_layer_count;
            u32                     available_EXT_count;
        };

        struct vk_physical_device_enumeration_data {
            VkPhysicalDevice                    device[VK_MAX_COUNT_PHYSICAL_DEVICES];
            VkPhysicalDeviceProperties          property[VK_MAX_COUNT_PHYSICAL_DEVICES];
            VkPhysicalDeviceFeatures            feature[VK_MAX_COUNT_PHYSICAL_DEVICES];
            VkQueueFamilyProperties             families[VK_MAX_COUNT_PHYSICAL_DEVICES][VK_MAX_COUNT_QUEUE_FAMILY];
            union vk_device_queue_family_idx    family_indecies[VK_MAX_COUNT_PHYSICAL_DEVICES];
            u32                                 family_count[VK_MAX_COUNT_PHYSICAL_DEVICES];
            u32                                 device_count;
            slop_t                              _[4];
        }
        vk_physical_device_enumeration_data;
    };
}
GLOBAL *g = NULL;

//
// FUNCTIONS
//

static
void process_key(union win32_key key)
{
    const static struct {
        keymap_t left;
        keymap_t right;
        keymap_t up;
        keymap_t down;
        keymap_t quit;
    }
    keymap = {
        .left  = 0x1E,
        .right = 0x20,
        .up    = 0x11,
        .down  = 0x1F,
        .quit  = 0x01,
    };

    if (key.scan_code == keymap.left)
    {
        OutputDebugStringA("A LEFT\n");
    }
    else if (key.scan_code == keymap.right)
    {
        OutputDebugStringA("D RIGHT\n");
    }
    else if (key.scan_code == keymap.up)
    {
        OutputDebugStringA("W UP\n");
    }
    else if (key.scan_code == keymap.down)
    {
        OutputDebugStringA("S DOWN\n");
    }
    else if (key.scan_code == keymap.quit)
    {
        OutputDebugStringA("ESC QUIT\n");
        g_running = 0x0;
    }
}

static
LRESULT CALLBACK window_procedure(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
    ASSERT(window);

    switch (message)
    {
        default: return DefWindowProcA(window, message, wparam, lparam);

        case WM_DESTROY:
        case WM_CLOSE:
        {
            g_running = 0x0;
        }
        break;
    }

    return 0;
}

static
void process_messages(void)
{
    MSG message;
    while (PeekMessageA(&message, NULL, 0x0, 0x0, PM_REMOVE))
    {
        switch (message.message)
        {
            case WM_QUIT: g_running = 0x0; break;

            default:
            {
#if !FIXME(DISABLE_TRANSLATE_MESSAGE)
                (void)TranslateMessage(&message);
#endif
                (void)DispatchMessageA(&message);
            }
            break;

            case WM_KEYDOWN:
            case WM_KEYUP:
            case WM_SYSKEYDOWN:
            case WM_SYSKEYUP:
            {
                union win32_key key = {
                    .whole = message.lParam,
                };
                process_key(key);
            }
            break;
        }
    }
}

static
VkBool32 vk_dbg_callback(VkDebugUtilsMessageSeverityFlagBitsEXT           severity,
                         VkDebugUtilsMessageTypeFlagsEXT                  type,
                         const VkDebugUtilsMessengerCallbackDataEXT*      data,
                         void*                                            user_data)
{
    UNREFERENCED_PARAMETER(severity);
    UNREFERENCED_PARAMETER(type);
    UNREFERENCED_PARAMETER(user_data);

#if 0
    switch (severity)
    {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
        default: break;
    }
#endif

    dbg_print("[VLAYER]: %s\n", data->pMessage);
    return VK_FALSE;
}


const GLOBAL VkDebugUtilsMessengerCreateInfoEXT g_vk_messenger_ci = {
    .sType           = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
    .pNext           = NULL,
    .flags           = 0x0,
    .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT|
                       VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT|
                       VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT  |
                       VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT   ,
    .messageType     = VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT|
                       VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT           |
                       VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT            |
                       VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT               ,
    .pfnUserCallback = &vk_dbg_callback,
    .pUserData       = NULL,
};

#if FIXME(DO_I_HAVE_TO_DESTROY_VK_DBG_MESSENGER)
PFN_vkDestroyDebugUtilsMessengerEXT g_vk_destroy_dbg_messenger = NULL;
#endif

static
VkDebugUtilsMessengerEXT vk_setup_debug_callback(VkInstance instance)
{
    ASSERT(instance);

    VkDebugUtilsMessengerEXT messenger;
    {
        VK_GET_PROC_ADDR(vkCreateDebugUtilsMessengerEXT, create_fn, instance);
        if (!create_fn)
        {
            dbgprint("Could not get %s address", _pfn_name_);
            return VK_NULL_HANDLE;
        }

        VkResult result = create_fn(instance, &g_vk_messenger_ci, NULL, &messenger);
        if (result != VK_SUCCESS)
        {
            dbgprint("Could not create debug messenger: %s", vk_errstr(result));
            return VK_NULL_HANDLE;
        }
    }

#if FIXME(DO_I_HAVE_TO_DESTROY_VK_DBG_MESSENGER)
    {
        VK_GET_PROC_ADDR(vkDestroyDebugUtilsMessengerEXT, destroy_fn, instance);
        if (!destroy_fn)
        {
            dbgprint("Could not get %s address", _pfn_name_);
            return VK_NULL_HANDLE;
        }

        g_vk_destroy_dbg_messenger = destroy_fn;
    }
#endif


    return messenger;
}

static
VkInstance vk_create_instance(u32 layer_count, literal_t layer_names[], u32 EXT_count, literal_t EXT_names[])
{
    const VkApplicationInfo app_info = {
        .sType                = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pNext                = NULL,
        .pApplicationName     = APPNAME,
        .applicationVersion   = VK_MAKE_VERSION(1,0,0),
        .pEngineName          = "No Engine",
        .engineVersion        = VK_MAKE_VERSION(1,0,0),
        .apiVersion           = VK_API_VERSION_1_3,
    };

    const VkInstanceCreateInfo instance_ci = {
        .sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext                   = &g_vk_messenger_ci,
        .flags                   = 0x0,
        .pApplicationInfo        = &app_info,
        .enabledLayerCount       = layer_count,
        .ppEnabledLayerNames     = layer_names,
        .enabledExtensionCount   = EXT_count,
        .ppEnabledExtensionNames = EXT_names,
    };

    VkInstance instance;
    {
        VkResult result = vkCreateInstance(&instance_ci, NULL, &instance);
        if (result != VK_SUCCESS)
        {
            dbgprint("Could not create Vulkan Instance: %s", vk_errstr(result));
            return VK_NULL_HANDLE;
        }
    }

    return instance;
}

static
void vk_enumerate_layers(VkLayerProperties layers[VK_MAX_COUNT_LAYERS], u32*const count)
{
    ASSERT(layers);
    ASSERT(count);

    *count = VK_MAX_COUNT_LAYERS;

    ASSERT_FN(VK_SUCCESS ==,
    vkEnumerateInstanceLayerProperties(count, layers)
    );

    for (u32 i = 0; i < *count; ++i)
    {
        dbgprint("[%u]: %s", i, layers[i].layerName);
    }
}

static
void vk_enumerate_EXTs(VkExtensionProperties EXTs[VK_MAX_COUNT_EXT], u32*const count)
{
    ASSERT(EXTs);
    ASSERT(count);

    *count = VK_MAX_COUNT_EXT;

    ASSERT_FN(VK_SUCCESS ==,
    vkEnumerateInstanceExtensionProperties(NULL, count, EXTs)
    );

    for (u32 i = 0; i < *count; ++i)
    {
        dbgprint("[%u]: %s", i, EXTs[i].extensionName);
    }
}

static
void vk_acquire_EXTs(VkExtensionProperties  available_EXT[VK_MAX_COUNT_LAYERS],
                     u32*                   available_EXT_count,
                     char                   names_buffer[BUFFER_SIZE_VK_EXT_NAMES],
                     char*                  EXT_names[VK_MAX_COUNT_EXT],
                     u32*                   EXT_count)
{
    ASSERT(available_EXT);
    ASSERT(names_buffer);
    ASSERT(EXT_names);
    ASSERT(EXT_count);

    vk_enumerate_EXTs(available_EXT, available_EXT_count);

    literal_t required_EXT[] = {
        VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
        VK_KHR_SURFACE_EXTENSION_NAME,
#if VK_USING_VALIDATION
        VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
#endif
    };

    const u32 required_EXT_count = sizeof_array(required_EXT);

    *EXT_count = 0;

    size_t offset = 0;
    for (u32 R = 0; R < required_EXT_count; ++R)
    {
        u32 found = 0x0;
        for (u32 A = 0; A < *available_EXT_count; ++A)
        {
            const size_t length = strlen(required_EXT[R]);
            if (!strncmp(available_EXT[A].extensionName, required_EXT[R], length))
            {
                const s64 size_left = BUFFER_SIZE_VK_EXT_NAMES - offset;
                ASSERT(size_left >= 0);

                EXT_names[*EXT_count] = &names_buffer[offset];
                {
                    strncpy_s(EXT_names[*EXT_count], size_left, available_EXT[A].extensionName, length);
                    {
                        dbgprint("Found EXT: [%u] %s", *EXT_count, EXT_names[*EXT_count]);
                    }

                    ++*EXT_count;
                }

                offset += length + 1;

                found = 0x1;
                break;
            }
        }

#if FIXME(DO_IF_REQUIRED_EXT_NOT_FOUND)
        ASSERT(found);
#endif
    }
}

static
void vk_acquire_layers(VkLayerProperties    available_layer[VK_MAX_COUNT_LAYERS],
                       u32*                 available_layer_count,
                       char                 names_buffer[BUFFER_SIZE_VK_LAYER_NAMES],
                       char*                layer_names[VK_MAX_COUNT_LAYERS],
                       u32*                 layer_count)
{
    ASSERT(available_layer);
    ASSERT(names_buffer);
    ASSERT(layer_names);
    ASSERT(layer_count);

    vk_enumerate_layers(available_layer, available_layer_count);

    literal_t required_layers[] = {
        "VK_LAYER_KHRONOS_validation",
    };

    const u32 required_layers_count = sizeof_array(required_layers);

    *layer_count = 0;

    size_t offset = 0;
    for (u32 R = 0; R < required_layers_count; ++R)
    {
        u32 found = 0x0;
        for (u32 A = 0; A < *available_layer_count; ++A)
        {
            const size_t length = strlen(required_layers[R]);
            if (!strncmp(available_layer[A].layerName, required_layers[R], length))
            {
                const s64 size_left = BUFFER_SIZE_VK_LAYER_NAMES - offset;
                ASSERT(size_left >= 0);

                layer_names[*layer_count] = &names_buffer[offset];
                {
                    strncpy_s(layer_names[*layer_count], size_left, available_layer[A].layerName, length);
                    {
                        dbgprint("Found Layer: [%u] %s", *layer_count, layer_names[*layer_count]);
                    }

                    ++*layer_count;
                }

                offset += length + 1;

                found = 0x1;
                break;
            }
        }

#if FIXME(DO_IF_REQUIRED_LAYER_NOT_FOUND)
        ASSERT(found);
#endif
    }

}


static inline
VkBool32 vk_is_family_index_valid(family_index_t index)
{
    return index < VK_MAX_COUNT_QUEUE_FAMILY;
}

static
void dbg_print_VkQueueFlagBits(VkQueueFlagBits flags)
{
    if (flags & VK_QUEUE_GRAPHICS_BIT)
    {
        dbgprint("%s", string_VkQueueFlagBits(VK_QUEUE_GRAPHICS_BIT));
    }
    if (flags & VK_QUEUE_COMPUTE_BIT)
    {
        dbgprint("%s", string_VkQueueFlagBits(VK_QUEUE_COMPUTE_BIT));
    }
    if (flags & VK_QUEUE_TRANSFER_BIT)
    {
        dbgprint("%s", string_VkQueueFlagBits(VK_QUEUE_TRANSFER_BIT));
    }
    if (flags & VK_QUEUE_SPARSE_BINDING_BIT)
    {
        dbgprint("%s", string_VkQueueFlagBits(VK_QUEUE_SPARSE_BINDING_BIT));
    }
    if (flags & VK_QUEUE_PROTECTED_BIT)
    {
        dbgprint("%s", string_VkQueueFlagBits(VK_QUEUE_PROTECTED_BIT));
    }
    if (flags & VK_QUEUE_VIDEO_DECODE_BIT_KHR)
    {
        dbgprint("%s", string_VkQueueFlagBits(VK_QUEUE_VIDEO_DECODE_BIT_KHR));
    }
    if (flags & VK_QUEUE_OPTICAL_FLOW_BIT_NV)
    {
        dbgprint("%s", string_VkQueueFlagBits(VK_QUEUE_OPTICAL_FLOW_BIT_NV));
    }
#ifdef VK_ENABLE_BETA_EXTENSIONS
    if (flags & VK_QUEUE_VIDEO_ENCODE_BIT_KHR)
    {
        dbgprint("%s", string_VkQueueFlagBits(VK_QUEUE_VIDEO_ENCODE_BIT_KHR));
    }
#endif
}

static inline
VkBool32 vk_is_PDEV_suitable(        VkPhysicalDevice            device,
                                     VkSurfaceKHR                surface,
                             const   VkPhysicalDeviceProperties* property,
                             const   VkPhysicalDeviceFeatures*   feature,
                             const   VkQueueFamilyProperties     family[VK_MAX_COUNT_QUEUE_FAMILY],
                             union   vk_device_queue_family_idx* family_indecies,
                             const   u32                         family_count)
{
    ASSERT(property);
    ASSERT(feature);
    ASSERT(family);
    ASSERT(family_count < VK_MAX_COUNT_QUEUE_FAMILY);

    VkBool32 device_ok = (
            property->deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU
            &&
            feature->geometryShader
    );

    if (!device_ok)
    {
        return VK_FALSE;
    }

    {
#if DEBUG
        for (u32 i = 0; i < family_count; ++i)
        {
            VkBool32 presenting_supported = VK_FALSE;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presenting_supported);

            dbgprint("Queue Family[%u]: can present = %s", i, presenting_supported == VK_TRUE ? "true" : "false");
            dbg_print_VkQueueFlagBits(family[i].queueFlags);
        }
#endif

        for (u32 i = 0; i < family_count; ++i)
        {
            VkBool32 got_graphics = vk_is_family_index_valid(family_indecies->graphics);
            if (!got_graphics)
            {
                if (family[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
                {
                    family_indecies->graphics = i;

                    got_graphics = VK_TRUE;
                }
            }

            VkBool32 got_present = vk_is_family_index_valid(family_indecies->present);
            if (!got_present)
            {
                VkBool32 supported = VK_FALSE;
                vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &supported);
                if (supported && (i != family_indecies->graphics))
                {
                    family_indecies->present = i;

                    got_present = VK_TRUE;
                }
            }

            VkBool32 done = (got_present && got_graphics);
            if (done)
            {
                return VK_TRUE;
            }
        }
    }

    return VK_FALSE;
}

static
VkPhysicalDevice vk_acquire_PDEV(VkInstance                                  instance,
                                 struct vk_physical_device_enumeration_data* data,
                                 struct vk_stored_PDEV_familes*              store,
                                 VkSurfaceKHR                                surface)
{
    ASSERT(instance);
    ASSERT(data);
    ASSERT(store);

    // Enumerating Physical Devices
    {
        data->device_count = VK_MAX_COUNT_PHYSICAL_DEVICES;

        ASSERT_FN(VK_SUCCESS ==,
        vkEnumeratePhysicalDevices(instance, &data->device_count, data->device)
        );
    }

    for (u32 i = 0; i < data->device_count; ++i)
    {
        VkPhysicalDevice*                          device = &data->device[i];
        VkPhysicalDeviceProperties*            properties = &data->property[i];
        VkPhysicalDeviceFeatures*                features = &data->feature[i];
        union vk_device_queue_family_idx* family_indecies = &data->family_indecies[i];
        {
            memset(family_indecies, 0xFF, sizeof(union vk_device_queue_family_idx));
        }

        vkGetPhysicalDeviceProperties(*device, properties);
        vkGetPhysicalDeviceFeatures(*device,   features);

        VkQueueFamilyProperties* families_per_device = data->families[i];
        {
            data->family_count[i] = VK_MAX_COUNT_QUEUE_FAMILY;

            vkGetPhysicalDeviceQueueFamilyProperties(*device, &data->family_count[i], families_per_device);
        }

        if (vk_is_PDEV_suitable(*device, surface, properties, features, families_per_device, family_indecies, data->family_count[i]))
        {
            // Storing family data for chosen Physical Device
            {
                store->count = data->family_count[i];
                store->indecies = *family_indecies;
                memcpy(store->property, families_per_device, sizeof(store->property));
            }

            return *device;
        }
    }

    dbgprint("Could not find suitable Physical Device");
    return VK_NULL_HANDLE;
}


static
VkDevice vk_acquire_LDEV(const union vk_device_queue_family_idx indecies, VkPhysicalDevice PDEV)
{
    ASSERT(PDEV);
    ASSERT(indecies.graphics < g->PDEV_familes.count);


    const float graphics_queue_prio[] = {
        [0] = 1.0f,
    };

    const float present_queue_prio[] = {
        [0] = 1.0f,
    };

    const u32 graphics_queue_count = sizeof_array(graphics_queue_prio);
    {
        ASSERT(graphics_queue_count < g->PDEV_familes.property[g->PDEV_familes.indecies.graphics].queueCount);
    }


    const u32 present_queue_count = sizeof_array(present_queue_prio);
    {
        ASSERT(present_queue_count < g->PDEV_familes.property[g->PDEV_familes.indecies.present].queueCount);
    }

    const VkDeviceQueueCreateInfo queue_ci[] = {
        [QUEUE_INDEX_GRAPHICS] = {
            .sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .pNext            = NULL,
            .flags            = 0x0,
            .queueFamilyIndex = indecies.graphics,
            .queueCount       = graphics_queue_count,
            .pQueuePriorities = graphics_queue_prio,
        },
        [QUEUE_INDEX_PRESENT] = {
            .sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .pNext            = NULL,
            .flags            = 0x0,
            .queueFamilyIndex = indecies.present,
            .queueCount       = present_queue_count,
            .pQueuePriorities = present_queue_prio,
        },
    };

    const VkPhysicalDeviceFeatures device_features = {
        .robustBufferAccess            			 = VK_FALSE,
        .fullDrawIndexUint32            		 = VK_FALSE,
        .imageCubeArray            				 = VK_FALSE,
        .independentBlend            			 = VK_FALSE,
        .geometryShader            				 = VK_FALSE,
        .tessellationShader            			 = VK_FALSE,
        .sampleRateShading            			 = VK_FALSE,
        .dualSrcBlend            				 = VK_FALSE,
        .logicOp            					 = VK_FALSE,
        .multiDrawIndirect            			 = VK_FALSE,
        .drawIndirectFirstInstance            	 = VK_FALSE,
        .depthClamp            					 = VK_FALSE,
        .depthBiasClamp            				 = VK_FALSE,
        .fillModeNonSolid            			 = VK_FALSE,
        .depthBounds            				 = VK_FALSE,
        .wideLines            					 = VK_FALSE,
        .largePoints            				 = VK_FALSE,
        .alphaToOne            					 = VK_FALSE,
        .multiViewport            				 = VK_FALSE,
        .samplerAnisotropy            			 = VK_FALSE,
        .textureCompressionETC2            		 = VK_FALSE,
        .textureCompressionASTC_LDR            	 = VK_FALSE,
        .textureCompressionBC            		 = VK_FALSE,
        .occlusionQueryPrecise            		 = VK_FALSE,
        .pipelineStatisticsQuery            	 = VK_FALSE,
        .vertexPipelineStoresAndAtomics          = VK_FALSE,
        .fragmentStoresAndAtomics            	 = VK_FALSE,
        .shaderTessellationAndGeometryPointSize  = VK_FALSE,
        .shaderImageGatherExtended            	 = VK_FALSE,
        .shaderStorageImageExtendedFormats       = VK_FALSE,
        .shaderStorageImageMultisample           = VK_FALSE,
        .shaderStorageImageReadWithoutFormat     = VK_FALSE,
        .shaderStorageImageWriteWithoutFormat    = VK_FALSE,
        .shaderUniformBufferArrayDynamicIndexing = VK_FALSE,
        .shaderSampledImageArrayDynamicIndexing  = VK_FALSE,
        .shaderStorageBufferArrayDynamicIndexing = VK_FALSE,
        .shaderStorageImageArrayDynamicIndexing  = VK_FALSE,
        .shaderClipDistance            			 = VK_FALSE,
        .shaderCullDistance            			 = VK_FALSE,
        .shaderFloat64            				 = VK_FALSE,
        .shaderInt64            				 = VK_FALSE,
        .shaderInt16            				 = VK_FALSE,
        .shaderResourceResidency            	 = VK_FALSE,
        .shaderResourceMinLod            		 = VK_FALSE,
        .sparseBinding            				 = VK_FALSE,
        .sparseResidencyBuffer            		 = VK_FALSE,
        .sparseResidencyImage2D            		 = VK_FALSE,
        .sparseResidencyImage3D            		 = VK_FALSE,
        .sparseResidency2Samples            	 = VK_FALSE,
        .sparseResidency4Samples            	 = VK_FALSE,
        .sparseResidency8Samples            	 = VK_FALSE,
        .sparseResidency16Samples            	 = VK_FALSE,
        .sparseResidencyAliased            		 = VK_FALSE,
        .variableMultisampleRate            	 = VK_FALSE,
        .inheritedQueries            			 = VK_FALSE,
    };

    const VkDeviceCreateInfo device_ci = {
        .sType            		 = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext            		 = NULL,
        .flags            		 = 0x0,
        .queueCreateInfoCount    = sizeof_array(queue_ci),
        .pQueueCreateInfos       = queue_ci,
#if FIXME(NO_NEED_TO_SEPARATE_BETWEEN_VULKAN_DEVICE_AND_VULKAN_INSTANCE)
        .enabledLayerCount       = 0,
        .ppEnabledLayerNames     = NULL,
#else
#   pragma message("warning: Should do something about it please")
#endif
        .enabledExtensionCount   = 0,
        .ppEnabledExtensionNames = NULL,

        .pEnabledFeatures        = &device_features,
    };


    VkDevice device = VK_NULL_HANDLE;
    {
        VkResult result = vkCreateDevice(PDEV, &device_ci, NULL, &device);
        if (result != VK_SUCCESS)
        {
            dbgprint("Could not create a device: %s", vk_errstr(result));
            ExitProcess(EXIT_FAILURE);
        }
    }

    return device;
}

static 
VkSurfaceKHR vk_create_surface(HINSTANCE hinstance, HWND window, VkInstance instance)
{
    const VkWin32SurfaceCreateInfoKHR win32_surface_ci = {
        .sType     = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
        .pNext     = NULL,
        .flags     = 0x0,
        .hinstance = hinstance,
        .hwnd      = window,
    };

    VkSurfaceKHR surface = VK_NULL_HANDLE;
    {
        ASSERT_FN(VK_SUCCESS ==,
        vkCreateWin32SurfaceKHR(instance, &win32_surface_ci, NULL, &surface)
        );
    }

    return surface;
}

//
// MAIN / ENTRY
//

#define ERROR_MAIN_RETURN(__FMT__, ...)                                                         \
    do                                                                                          \
    {                                                                                           \
        dbgprint("[ERROR_MAIN_RETURN] (%s:%d) "__FMT__"\n", __FILE__,__LINE__, ##__VA_ARGS__);  \
        return EXIT_FAILURE;                                                                    \
    }                                                                                           \
    while(0)

int WinMain(_In_     HINSTANCE instance,
            _In_opt_ HINSTANCE _prev_instance,
            _In_     LPSTR     _cmdline,
            _In_     int       _showopts)
{
    UNREFERENCED_PARAMETER(instance);
    UNREFERENCED_PARAMETER(_prev_instance);
    UNREFERENCED_PARAMETER(_cmdline);
    UNREFERENCED_PARAMETER(_showopts);


    g = VirtualAlloc(0x0,
                     sizeof(struct global_memory),
                     MEM_COMMIT|MEM_RESERVE,
                     PAGE_READWRITE);
    if (!g)
    {
        ERROR_MAIN_RETURN("Could not allocate %zu bytes: %s", sizeof(struct global_memory),
                                                              win32_errstr(GetLastError()));
    }

    vk_acquire_EXTs(g->available_EXT,
                   &g->available_EXT_count,
                   g->EXT_names_buffer,
                   g->EXT_names,
                   &g->EXT_count);
    vk_acquire_layers(g->available_layer,
                      &g->available_layer_count,
                      g->layer_names_buffer,
                      g->layer_names,
                      &g->layer_count);


    HWND window = create_window(instance);
    if (!window)
    {
        dbgprint("Could not create a window: %s", win32_errstr(GetLastError()));
        return EXIT_FAILURE;
    }

    VkInstance vk_instance = vk_create_instance(g->layer_count,(literal_t*)g->layer_names,
                                                g->EXT_count,  (literal_t*)g->EXT_names);
    if (vk_instance == VK_NULL_HANDLE)
    {
        ERROR_MAIN_RETURN("vk_create_instance()");
    }

    VkDebugUtilsMessengerEXT vk_dbg_messenger = vk_setup_debug_callback(vk_instance);
    if (vk_dbg_messenger == VK_NULL_HANDLE)
    {
        ERROR_MAIN_RETURN("vk_setup_debug_callback()");
    }

    VkSurfaceKHR vk_surface = vk_create_surface(instance, window, vk_instance);
    if (vk_instance == VK_NULL_HANDLE)
    {
        ERROR_MAIN_RETURN("vk_create_surface()");
    }

    VkPhysicalDevice vk_PDEV = vk_acquire_PDEV(vk_instance, &g->vk_physical_device_enumeration_data, &g->PDEV_familes, vk_surface);
    if (vk_PDEV == VK_NULL_HANDLE)
    {
        ERROR_MAIN_RETURN("vk_acquire_PDEV()");
    }

    ASSERT(g->PDEV_familes.indecies.graphics != g->PDEV_familes.indecies.present);

    VkDevice vk_device = vk_acquire_LDEV(g->PDEV_familes.indecies, vk_PDEV);
    if (vk_device == VK_NULL_HANDLE)
    {
        ERROR_MAIN_RETURN("vk_acquire_LDEV()");
    }


    // Getting queues
    {
        vkGetDeviceQueue(vk_device, g->PDEV_familes.indecies.graphics, 0, &g->graphics_queue);
        {
            ASSERT(g->graphics_queue);
        }

        vkGetDeviceQueue(vk_device, g->PDEV_familes.indecies.present, 0, &g->present_queue);
        {
            ASSERT(g->present_queue);
        }
    }

    while(g_running)
    {
        process_messages();
    }

#if FIXME(DO_I_HAVE_TO_DESTROY_VK_DBG_MESSENGER)
    ASSERT(vk_dbg_messenger);
    g_vk_destroy_dbg_messenger(vk_instance, vk_dbg_messenger, NULL);

    vkDestroyInstance(vk_instance, NULL);
#endif

    return EXIT_SUCCESS;
}

