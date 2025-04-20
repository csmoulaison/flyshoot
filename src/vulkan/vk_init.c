void vk_create_swapchain(
	struct vk_context* vk, 
	bool               recreate)
{
	if(recreate) 
	{
		vkDeviceWaitIdle(vk->device);
		for(uint32_t i = 0; i < vk->swap_images_len; i++)
		{
			vkDestroyImageView(vk->device, vk->swap_views[i], 0);
		}
		vkDestroySwapchainKHR(vk->device, vk->swapchain, 0);

		vkDestroyImage(vk->device, vk->render_image, 0);
		vkDestroyImageView(vk->device, vk->render_view, 0);
		vkFreeMemory(vk->device, vk->render_image_memory, 0);

		vkDestroySemaphore(vk->device, vk->semaphore_image_available, 0);
		vkDestroySemaphore(vk->device, vk->semaphore_render_finished, 0);
	}

	// Query surface capabilities.
	uint32_t image_count = 0;
	VkSurfaceTransformFlagBitsKHR pre_transform;
	{
		VkSurfaceCapabilitiesKHR abilities;

		VK_VERIFY(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vk->physical_device, vk->surface, &abilities));

		// XXX - See what we can do about this, or whether it's necessary to do
		// anything different.
		vk->swap_extent.width = abilities.maxImageExtent.width;
		vk->swap_extent.height = abilities.maxImageExtent.height;

		if(abilities.minImageExtent.width  > vk->swap_extent.width
		|| abilities.maxImageExtent.width  < vk->swap_extent.width
		|| abilities.minImageExtent.height > vk->swap_extent.height
		|| abilities.maxImageExtent.height < vk->swap_extent.height) 
		{
			printf("Surface KHR extents are not compatible with configured surface sizes.\n");
			PANIC();
		}

		image_count = abilities.minImageCount + 1;
		if(abilities.maxImageCount > 0 && image_count > abilities.maxImageCount) 
		{
			image_count = abilities.maxImageCount;
		}

		pre_transform = abilities.currentTransform;
	}

	// Choose surface format.
	{
		uint32_t formats_len;
		vkGetPhysicalDeviceSurfaceFormatsKHR(vk->physical_device, vk->surface, &formats_len, 0);
		if(formats_len == 0) 
		{
			printf("Physical device doesn't support any formats?\n");
			PANIC();
		}

		VkSurfaceFormatKHR formats[formats_len];
		vkGetPhysicalDeviceSurfaceFormatsKHR(vk->physical_device, vk->surface, &formats_len, formats);

		vk->surface_format = formats[0];
		for(int i = 0; i < formats_len; i++) 
		{
			if(formats[i].format == VK_FORMAT_B8G8R8A8_SRGB && formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) 
			{
				vk->surface_format = formats[i];
				break;
			}
		}
	}

	// Choose presentation mode.
	// 
	// Default to VK_PRESENT_MODE_FIFO_KHR, as this is the only mode required to
	// be supported by the spec.
	VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR;
	{
		uint32_t modes_len;
		VK_VERIFY(vkGetPhysicalDeviceSurfacePresentModesKHR(vk->physical_device, vk->surface, &modes_len, 0));

		VkPresentModeKHR modes[modes_len];
		VK_VERIFY(vkGetPhysicalDeviceSurfacePresentModesKHR(vk->physical_device, vk->surface, &modes_len, modes));

		for(int i = 0; i < modes_len; i++) 
		{
			if(modes[i] == VK_PRESENT_MODE_MAILBOX_KHR) 
			{
				present_mode = modes[i];
				break;
			}
		}
	}

	VkSwapchainCreateInfoKHR info = {};
	info.sType                 = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	info.pNext                 = 0;
	info.flags                 = 0; // TODO mutable format or any other flags?
	info.surface               = vk->surface;
	info.minImageCount         = image_count; // TODO get this value.
	info.imageFormat           = vk->surface_format.format;
	info.imageColorSpace       = vk->surface_format.colorSpace;
	info.imageExtent           = vk->swap_extent;
	info.imageArrayLayers      = 1;
	info.imageUsage            = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; // TODO probably right, but we'll see.
	info.imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE; // TODO needs to be CONCURRENT if compute is in different family from present
	info.queueFamilyIndexCount = 0; // Not used in exclusive mode. Need to check for concurrent.
	info.pQueueFamilyIndices   = 0; // Also not used in exclusive mode, see above.
	info.preTransform          = pre_transform;
	info.compositeAlpha        = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	info.presentMode           = present_mode;
	info.clipped               = VK_TRUE;
	info.oldSwapchain          = VK_NULL_HANDLE;

#if VK_IMMEDIATE
		info.presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
#endif

	VK_VERIFY(vkCreateSwapchainKHR(vk->device, &info, 0, &vk->swapchain));
	VK_VERIFY(vkGetSwapchainImagesKHR(vk->device, vk->swapchain, &vk->swap_images_len, 0));
	VK_VERIFY(vkGetSwapchainImagesKHR(vk->device, vk->swapchain, &vk->swap_images_len, vk->swap_images));

	// Create image views.
	for(int i = 0; i < vk->swap_images_len; i++) 
	{
		vk_create_image_view(
			vk->device, 
			&vk->swap_views[i], 
			vk->swap_images[i], 
			vk->surface_format.format, 
			VK_IMAGE_ASPECT_COLOR_BIT);
	}

	// Create render image resources for multisampling
	vk_allocate_image_and_view(
		vk->device, 
		vk->physical_device, 
		&vk->render_image,
		&vk->render_image_memory,
		&vk->render_view,
		vk->swap_extent.width,
		vk->swap_extent.height,
		vk->surface_format.format,
		vk->render_samples,
		VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
		VK_IMAGE_ASPECT_COLOR_BIT);

	// Create image resources for depth buffering
	vk_allocate_image_and_view(
		vk->device, 
		vk->physical_device, 
		&vk->depth_image,
		&vk->depth_image_memory,
		&vk->depth_view,
		vk->swap_extent.width,
		vk->swap_extent.height,
		DEPTH_ATTACHMENT_FORMAT,
		vk->render_samples,
		VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
		VK_IMAGE_ASPECT_DEPTH_BIT);

	// Create synchronization primitives
	{
		VkSemaphoreCreateInfo semaphore_info = {};
		semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VK_VERIFY(vkCreateSemaphore(vk->device, &semaphore_info, 0, &vk->semaphore_image_available));
		VK_VERIFY(vkCreateSemaphore(vk->device, &semaphore_info, 0, &vk->semaphore_render_finished));
	}
}

struct vk_context vk_init(struct vk_platform* platform)
{
	struct vk_context vk;

	// Create instance.
	{
		VkApplicationInfo app = {};
		app.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		app.pNext              = 0;
		app.pApplicationName   = PROGRAM_NAME;
		app.applicationVersion = 1;
		app.pEngineName        = 0;
		app.engineVersion      = 0;
		app.apiVersion         = VK_API_VERSION_1_3;

		uint32_t exts_len = platform->window_extensions_len;

#if VK_DEBUG
			exts_len++;
			char* debug_ext = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
#endif

		const char* exts[exts_len];
		for(int i = 0; i < platform->window_extensions_len; i++) {
			exts[i] = platform->window_extensions[i];
		}

#if VK_DEBUG
			exts[exts_len - 1] = debug_ext;
#endif

		VkInstanceCreateInfo info = {};
		info.sType            = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		info.pNext            = 0;
		info.flags            = 0;
		info.pApplicationInfo = &app;

		uint32_t layers_len = 0;

		#ifdef VK_DEBUG
			layers_len++;
		#endif

		const char* layers[layers_len];

#if VK_DEBUG
			layers[layers_len - 1] = "VK_LAYER_KHRONOS_validation";
#endif

		info.enabledLayerCount = layers_len;
		info.ppEnabledLayerNames = layers;

		info.enabledExtensionCount = exts_len;
		info.ppEnabledExtensionNames = exts;

		VK_VERIFY(vkCreateInstance(&info, 0, &vk.instance));
	}

	// Create surface.
	{
		VK_VERIFY(platform->create_surface_callback(&vk, platform->context));
	}

	// Create physical device.
	uint32_t graphics_family_idx = 0;
	{
		uint32_t devices_len;

		VK_VERIFY(vkEnumeratePhysicalDevices(vk.instance, &devices_len, 0));

		VkPhysicalDevice devices[devices_len];

		VK_VERIFY(vkEnumeratePhysicalDevices(vk.instance, &devices_len, devices));

		vk.physical_device = 0;
		for(int i = 0; i < devices_len; i++) 
		{
			// Check queue families
			uint32_t fams_len;
			vkGetPhysicalDeviceQueueFamilyProperties(devices[i], &fams_len, 0);
			VkQueueFamilyProperties fams[fams_len];
			vkGetPhysicalDeviceQueueFamilyProperties(devices[i], &fams_len, fams);

			//bool compute = false;
			bool graphics = false;
			for(int j = 0; j < fams_len; j++) 
			{
				/*
				if(fams[j].queueFlags & VK_QUEUE_COMPUTE_BIT) 
				{
					compute = true;
					compute_family_idx = j;
				}
				*/
				if(fams[j].queueFlags & VK_QUEUE_GRAPHICS_BIT) 
				{
					graphics = true;
					graphics_family_idx = j;
				}
			}
			if(!graphics) 
			{
				continue;
			}

			// Check device extensions.
			uint32_t exts_len;
			vkEnumerateDeviceExtensionProperties(devices[i], 0, &exts_len, 0);
			VkExtensionProperties exts[exts_len];
			vkEnumerateDeviceExtensionProperties(devices[i], 0, &exts_len, exts);

			bool swapchain = false;
			bool dynamic = false;
			for(int j = 0; j < exts_len; j++) 
			{
				if(strcmp(exts[j].extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0) 
				{
					swapchain = true;
					continue;
				}
				if(strcmp(exts[j].extensionName, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME) == 0) 
				{
					dynamic = true;
					continue;
				}
			}
			if(!swapchain || !dynamic) 
			{
				continue;
			}

			vk.physical_device = devices[i];

			VkPhysicalDeviceProperties properties;
			vkGetPhysicalDeviceProperties(vk.physical_device, &properties);
			VkSampleCountFlags sample_counts = properties.limits.framebufferColorSampleCounts; //& properties.limits.framebufferDepthSampleCounts;
			if(sample_counts & VK_SAMPLE_COUNT_64_BIT) 
			{ 
				vk.render_samples = VK_SAMPLE_COUNT_64_BIT; 
			} 
			else if(sample_counts & VK_SAMPLE_COUNT_32_BIT)
			{
				vk.render_samples = VK_SAMPLE_COUNT_32_BIT; 
			}
			else if(sample_counts & VK_SAMPLE_COUNT_16_BIT)
			{
				vk.render_samples = VK_SAMPLE_COUNT_16_BIT; 
			}
			else if(sample_counts & VK_SAMPLE_COUNT_8_BIT)
			{
				vk.render_samples = VK_SAMPLE_COUNT_8_BIT; 
			}
			else if(sample_counts & VK_SAMPLE_COUNT_4_BIT)
			{
				vk.render_samples = VK_SAMPLE_COUNT_4_BIT; 
			}
			else if(sample_counts & VK_SAMPLE_COUNT_2_BIT)
			{
				vk.render_samples = VK_SAMPLE_COUNT_2_BIT; 
			}
			else
			{
				vk.render_samples = VK_SAMPLE_COUNT_1_BIT;
			}
		}

		// Exit if we haven't found an eligible device.
		if(!vk.physical_device) 
		{
			printf("No suitable physical device.\n");
			PANIC();
		}
	}

	// Create logical device.
	{ 
		VkDeviceQueueCreateInfo queue = {};
		queue.sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queue.pNext            = 0;
		queue.flags            = 0;
		queue.queueFamilyIndex = graphics_family_idx;
		queue.queueCount       = 1; // 1 because only 1 queue, right?
		float priority         = 1.0f;
	 	queue.pQueuePriorities = &priority;

	 	VkPhysicalDeviceDynamicRenderingFeatures dynamic_features = {};
		dynamic_features.sType            = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES;
		dynamic_features.pNext            = 0;
		dynamic_features.dynamicRendering = VK_TRUE;


		VkPhysicalDeviceFeatures2 features = {};
		features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
		features.pNext = &dynamic_features;
		vkGetPhysicalDeviceFeatures2(vk.physical_device, &features);
		
		VkDeviceCreateInfo info = {};
		info.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	 	info.pNext                   = &features;
	 	info.flags                   = 0;
	 	info.queueCreateInfoCount    = 1;
	 	info.pQueueCreateInfos       = &queue;
	 	const char* device_exts[2] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME };
	 	info.enabledExtensionCount   = 2;
	 	info.ppEnabledExtensionNames = device_exts;
	 	VK_VERIFY(vkCreateDevice(vk.physical_device, &info, 0, &vk.device));

		vkGetDeviceQueue(vk.device, graphics_family_idx, 0, &vk.queue_graphics);
	}

	// Create swapchain, images, and image views. This has been abstracted to allow
	// swapchain recreation after initialization in the case of window resize, for
	// example.
	// XXX - If I was being really pedantic, we would also include pipeline
	// recreation as the surface format could theoretically also change at runtime.
	{
		vk_create_swapchain(&vk, false);
	}

	// Allocate host visible memory buffer.
	{
		VkDeviceSize buf_size = sizeof(struct vk_host_memory);

		vk_allocate_buffer(
			vk.device,
			vk.physical_device,
			&vk.host_visible_buffer,
			&vk.host_visible_memory,
			buf_size,
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		vkMapMemory(vk.device, vk.host_visible_memory, 0, buf_size, 0, (void*)&vk.host_visible_mapped);
	}

	// Create graphics pipelines
	{
		struct vk_descriptor_info world_descriptors[2];

		world_descriptors[0].type             = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		world_descriptors[0].offset_in_buffer = offsetof(struct vk_host_memory, global);
		world_descriptors[0].range_in_buffer  = sizeof(struct vk_ubo_global_world);

		world_descriptors[1].type             = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		world_descriptors[1].offset_in_buffer = offsetof(struct vk_host_memory, instance);
		world_descriptors[1].range_in_buffer  = sizeof(mat4);

		struct vk_attribute_description world_attributes[2];

		world_attributes[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		world_attributes[0].offset = offsetof(struct vk_cube_vertex, pos);

		world_attributes[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		world_attributes[1].offset = offsetof(struct vk_cube_vertex, color);

		VkShaderModule shader_world_vert = vk_create_shader_module(vk.device, "shaders/world_vert.spv");
		VkShaderModule shader_world_frag = vk_create_shader_module(vk.device, "shaders/world_frag.spv");

		vk_create_graphics_pipeline(
			&vk, 
			&vk.pipeline_resources_world, 
			world_descriptors, 
			2, 
			world_attributes,
			2,
			sizeof(struct vk_cube_vertex),
			shader_world_vert, 
			shader_world_frag);

		struct vk_descriptor_info reticle_descriptor;
		reticle_descriptor.type             = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		reticle_descriptor.offset_in_buffer = offsetof(struct vk_host_memory, global) + offsetof(struct vk_ubo_global, reticle_pos),
		reticle_descriptor.range_in_buffer  = sizeof(struct v2);

		struct vk_attribute_description reticle_attribute;
		reticle_attribute.format = VK_FORMAT_R32G32_SFLOAT;
		reticle_attribute.offset = offsetof(struct vk_reticle_vertex, pos);

		VkShaderModule shader_reticle_vert = vk_create_shader_module(vk.device, "shaders/reticle_vert.spv");
		VkShaderModule shader_reticle_frag = vk_create_shader_module(vk.device, "shaders/reticle_frag.spv");

		vk_create_graphics_pipeline(
			&vk, 
			&vk.pipeline_resources_reticle, 
			&reticle_descriptor, 
			1, 
			&reticle_attribute,
			1,
			sizeof(struct vk_reticle_vertex),
			shader_reticle_vert, 
			shader_reticle_frag);
	}

	// Create command pool and allocate command buffers
	{
		VkCommandPoolCreateInfo pool_info = {};
		pool_info.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		pool_info.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		pool_info.queueFamilyIndex = graphics_family_idx;
		VK_VERIFY(vkCreateCommandPool(vk.device, &pool_info, 0, &vk.command_pool));

		VkCommandBufferAllocateInfo buf_info = {};
		buf_info.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		buf_info.commandPool        = vk.command_pool;
		buf_info.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		buf_info.commandBufferCount = 1;
		VK_VERIFY(vkAllocateCommandBuffers(vk.device, &buf_info, &vk.command_buffer));
	}

	// Allocate device local memory buffer
	{
#define MESHES_LEN 2
		
		vk.mesh_data_cube.vertex_memory        = (void*)cube_vertices;
		vk.mesh_data_cube.index_memory         = (void*)cube_indices;
		vk.mesh_data_cube.vertex_stride        = sizeof(struct vk_cube_vertex);
		vk.mesh_data_cube.vertices_len         = CUBE_VERTICES_LEN;
		vk.mesh_data_cube.indices_len          = CUBE_INDICES_LEN;

		vk.mesh_data_reticle.vertex_memory     = (void*)reticle_vertices;
		vk.mesh_data_reticle.index_memory      = (void*)reticle_indices;
		vk.mesh_data_reticle.vertex_stride     = sizeof(struct vk_reticle_vertex);
		vk.mesh_data_reticle.vertices_len      = RETICLE_VERTICES_LEN;
		vk.mesh_data_reticle.indices_len       = RETICLE_INDICES_LEN;

		struct vk_mesh_data* mesh_datas[MESHES_LEN] = {&vk.mesh_data_cube, &vk.mesh_data_reticle};
		size_t mesh_vert_buffer_sizes[MESHES_LEN];
		size_t mesh_index_buffer_sizes[MESHES_LEN];

		size_t buf_size = 0;
		for(uint8_t i = 0; i < MESHES_LEN; i++)
		{
			mesh_vert_buffer_sizes[i]  = mesh_datas[i]->vertex_stride * mesh_datas[i]->vertices_len;
			mesh_index_buffer_sizes[i] = sizeof(uint16_t)             * mesh_datas[i]->indices_len;

			mesh_datas[i]->buffer_offset_vertex = buf_size;
			mesh_datas[i]->buffer_offset_index  = buf_size + mesh_vert_buffer_sizes[i];
			
			buf_size += mesh_vert_buffer_sizes[i] + mesh_index_buffer_sizes[i];
		}

		VkBuffer staging_buf;
		VkDeviceMemory staging_buf_mem;

		vk_allocate_buffer(
			vk.device,
			vk.physical_device,
			&staging_buf,
			&staging_buf_mem,
			buf_size, 
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		void* buf_data;
		vkMapMemory(vk.device, staging_buf_mem, 0, buf_size, 0, &buf_data);
		{
			size_t total_offset = 0;
			for(uint8_t i = 0; i < MESHES_LEN; i++)
			{
				memcpy(buf_data + mesh_datas[i]->buffer_offset_vertex, mesh_datas[i]->vertex_memory, mesh_vert_buffer_sizes[i]);
				memcpy(buf_data + mesh_datas[i]->buffer_offset_index,  mesh_datas[i]->index_memory,  mesh_index_buffer_sizes[i]);
				total_offset += mesh_vert_buffer_sizes[i] + mesh_index_buffer_sizes[i];
			}
		}
		vkUnmapMemory(vk.device, staging_buf_mem);

		vk_allocate_buffer(
			vk.device,
			vk.physical_device,
			&vk.device_local_buffer,
			&vk.device_local_memory,
			buf_size, 
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		// TODO - I'm copying this to do the texture image transition.
		// I have two options, not sure at this point what makes sense, and it's late
		// at night.
		//   (ref: multiple_command_buffers)
		// 
		// 1. Only ever allocate one command buffer, or just do another for
		// initialization. At any rate, I have no idea why I'm using a temp one for
		// this at the moment.
		// 2. If it makes sense to just be making temp command buffers like this, the
		// process should at least be made into a function.
		VkCommandBufferAllocateInfo alloc_info = {};
		alloc_info.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		alloc_info.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		alloc_info.commandPool        = vk.command_pool;
		alloc_info.commandBufferCount = 1;

		VkCommandBuffer cmd_buf;
		vkAllocateCommandBuffers(vk.device, &alloc_info, &cmd_buf);

		VkCommandBufferBeginInfo begin_info = {};
		begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(cmd_buf, &begin_info);
		{
			VkBufferCopy copy = {};
			copy.size = buf_size;
			vkCmdCopyBuffer(cmd_buf, staging_buf, vk.device_local_buffer, 1, &copy);
		}
		vkEndCommandBuffer(cmd_buf);

		VkSubmitInfo submit_info = {};
		submit_info.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit_info.commandBufferCount = 1;
		submit_info.pCommandBuffers    = &cmd_buf;

		// TODO - We are using the graphics queue for this presently, but might we
		// want to use a transfer queue for this?
		// I'm not sure if there's any possible performance boost, and I'd rather not
		// do it just for the sake of it.
		// 
		// High level on how this might be done at the following link:
		// https://vulkan-tutorial.com/Vertex_buffers/Staging_buffer
		vkQueueSubmit(vk.queue_graphics, 1, &submit_info, VK_NULL_HANDLE);
		vkQueueWaitIdle(vk.queue_graphics);

		vkFreeCommandBuffers(vk.device, vk.command_pool, 1, &cmd_buf);
		vkDestroyBuffer(vk.device, staging_buf, 0);
		vkFreeMemory(vk.device, staging_buf_mem, 0);
	}

	return vk;
}
