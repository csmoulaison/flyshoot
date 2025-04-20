#define VK_DEBUG 1
#define VK_IMMEDIATE 0

#define MAX_SWAP_IMAGES 4
#define MAX_IN_FLIGHT_FRAMES 2
#define DEPTH_ATTACHMENT_FORMAT VK_FORMAT_D32_SFLOAT

#include <vulkan/vulkan.h>

VkResult vk_verify_macro_result;

#define VK_VERIFY(FUNC) vk_verify_macro_result = FUNC;\
						if(vk_verify_macro_result != VK_SUCCESS)\
						{\
							printf("VK_VERIFY error (%i)\n", vk_verify_macro_result);\
							PANIC();\
						}\

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_BMP
#include "../extern/stb_image.h"

#include "vk_structs.c"
#include "vk_static_data.c"
#include "vk_helpers.c"
#include "vk_init.c"
#include "vk_loop.c"

