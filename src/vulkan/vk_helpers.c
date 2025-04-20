VkShaderModule vk_create_shader_module(VkDevice device, const char* fname)
{
	FILE* file = fopen(fname, "r");
	if(!file)
	{
		printf("Failed to open file: %s\n", fname);
		PANIC();
	}
	fseek(file, 0, SEEK_END);
	uint32_t fsize = ftell(file);
	fseek(file, 0, SEEK_SET);
	char src[fsize];

	char c;
	uint32_t i = 0;
	while((c = fgetc(file)) != EOF)
	{
		src[i] = c;
		i++;
	}
	fclose(file);
	
	VkShaderModuleCreateInfo info = {};
	info.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	info.codeSize = fsize;
	info.pCode    = (uint32_t*)src;

	VkShaderModule module;
	VkResult res = vkCreateShaderModule(device, &info, 0, &module);
	if(res != VK_SUCCESS)
	{
		printf("Error %i: Failed to create shader module.\n", res);
		PANIC();
	}
	
	return module;
}

uint32_t vk_get_memory_type(
	VkPhysicalDevice      physical_device, 
	uint32_t              type_filter, 
	VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties mem_properties;
	vkGetPhysicalDeviceMemoryProperties(physical_device, &mem_properties);
	
	for(uint32_t i = 0; i < mem_properties.memoryTypeCount; i++)
	{
		if((type_filter & (1 << i)) && (mem_properties.memoryTypes[i].propertyFlags & properties) == properties)
		{
			return i;
		}
	}

	printf("Failed to find suitable memory type for buffer.\n");
	PANIC();
}

void vk_allocate_buffer(
	VkDevice              device,
	VkPhysicalDevice      physical_device,
	VkBuffer*             buffer,
	VkDeviceMemory*       memory,
	VkDeviceSize          size, 
	VkBufferUsageFlags    usage, 
	VkMemoryPropertyFlags properties)
{
	VkBufferCreateInfo buf_info = {};
	buf_info.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buf_info.size        = size;
	buf_info.usage       = usage;
	buf_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VkResult res = vkCreateBuffer(device, &buf_info, 0, buffer);
	if(res != VK_SUCCESS)
	{
		printf("Error %i: Failed to create buffer.\n", res);
		PANIC();
	}

	VkMemoryRequirements mem_reqs;
	vkGetBufferMemoryRequirements(device, *buffer, &mem_reqs);

	VkPhysicalDeviceMemoryProperties mem_properties;
	vkGetPhysicalDeviceMemoryProperties(physical_device, &mem_properties);

	VkMemoryAllocateInfo alloc_info = {};
	alloc_info.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	alloc_info.allocationSize  = mem_reqs.size;
	alloc_info.memoryTypeIndex = vk_get_memory_type(
		physical_device,
		mem_reqs.memoryTypeBits, 
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	res = vkAllocateMemory(device, &alloc_info, 0, memory);
	if(res != VK_SUCCESS)
	{
		printf("Error %i: Failed to allocate buffer memory.\n", res);
	}
	vkBindBufferMemory(device, *buffer, *memory, 0);
}

void vk_allocate_image(
	VkDevice          device,
	VkPhysicalDevice  physical_device,
	VkImage*          image,
	VkDeviceMemory*   memory,
	uint32_t          width,
	uint32_t          height,
	VkFormat          format,
	uint32_t          samples,
	VkImageUsageFlags usage_mask)
{
	VkImageCreateInfo image_info = {};
	image_info.sType 		 = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	image_info.imageType     = VK_IMAGE_TYPE_2D;
	image_info.format        = format;
	image_info.extent.width  = width;
	image_info.extent.height = height;
	image_info.extent.depth  = 1;
	image_info.mipLevels     = 1;
	image_info.arrayLayers   = 1;
	image_info.samples       = samples;
	image_info.tiling        = VK_IMAGE_TILING_OPTIMAL;
	image_info.usage         = usage_mask;
	image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	VkResult res = vkCreateImage(device, &image_info, 0, image);
	if(res != VK_SUCCESS) 
	{
		printf("Error %i: Failed to create image.\n", res);
		PANIC();
	}

	VkMemoryRequirements mem_reqs = {};
	vkGetImageMemoryRequirements(device, *image, &mem_reqs);

	VkMemoryAllocateInfo alloc_info = {};
	alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	alloc_info.allocationSize = mem_reqs.size;
	alloc_info.memoryTypeIndex = vk_get_memory_type(
		physical_device,
		mem_reqs.memoryTypeBits, 
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	res = vkAllocateMemory(device, &alloc_info, 0, memory);
	if(res != VK_SUCCESS) 
	{
		printf("Error %i: Failed to allocate image memory..\n", res);
		PANIC();
	}
	res = vkBindImageMemory(device, *image, *memory, 0);
	if(res != VK_SUCCESS) 
	{
		printf("Error %i: Failed to bind image memory.\n", res);
		PANIC();
	}
}

void vk_create_image_view(
	VkDevice           device,
	VkImageView*       view,
	VkImage            image,
	VkFormat           format,
	VkImageAspectFlags aspect_mask)
{
	VkImageViewCreateInfo view_info = {};
	view_info.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	view_info.image                           = image;
	view_info.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
	view_info.format                          = format;
	view_info.subresourceRange.aspectMask     = aspect_mask;
	view_info.subresourceRange.baseMipLevel   = 0;
	view_info.subresourceRange.levelCount     = 1;
	view_info.subresourceRange.baseArrayLayer = 0;
	view_info.subresourceRange.layerCount     = 1;

	VkResult res = vkCreateImageView(device, &view_info, 0, view);
	if(res != VK_SUCCESS) 
	{
		printf("Error %i: Failed to create image view.\n", res);
		PANIC();
	}
}

void vk_allocate_image_and_view(
	VkDevice           device,
	VkPhysicalDevice   physical_device,
	VkImage*           image,
	VkDeviceMemory*    memory,
	VkImageView*       view,
	uint32_t           width,
	uint32_t           height,
	VkFormat           format,
	uint32_t           samples,
	VkImageUsageFlags  usage_mask,
	VkImageAspectFlags aspect_mask)
{
	vk_allocate_image(device, physical_device, image, memory, width, height, format, samples, usage_mask);
	vk_create_image_view(device, view, *image, format, aspect_mask);
}

void vk_allocate_texture(
	VkDevice         device,
	VkPhysicalDevice physical_device,
	VkImage*         image,
	VkDeviceMemory*  memory,
	char*            fname)
{
	int32_t tex_w;
	int32_t tex_h;
	int32_t tex_channels;
	stbi_uc* pixels = stbi_load(fname, &tex_w, &tex_h, &tex_channels, STBI_rgb_alpha);
	if(!pixels)
	{
		printf("Failed to load image file: %s.\n", fname);
		PANIC();
	}

	VkDeviceSize img_size = tex_w * tex_h * 4;
	VkBuffer staging_buf;
	VkDeviceMemory staging_mem;

	vk_allocate_buffer(
		device, 
		physical_device,
		&staging_buf, 
		&staging_mem,
		img_size,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	void* data;
	vkMapMemory(device, staging_mem, 0, img_size, 0, &data);
	memcpy(data, pixels, (size_t)img_size);
	vkUnmapMemory(device, staging_mem);

	stbi_image_free(pixels);

	vk_allocate_image(
		device,
		physical_device,
		image, 
		memory,
		tex_w,
		tex_h,
		VK_FORMAT_R8G8B8A8_SRGB,
		VK_SAMPLE_COUNT_1_BIT,
		VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
}

void vk_create_graphics_pipeline(
	struct vk_context*               vk,
	struct vk_pipeline_resources*    resources,
	struct vk_descriptor_info*       descriptor_infos,
	uint8_t                          descriptors_len,
	struct vk_attribute_description* attribute_descriptions,
	uint8_t                          attribute_descriptions_len,
	size_t                           vertex_stride,
	VkShaderModule                   shader_vert,
	VkShaderModule                   shader_frag)
{
	VkDescriptorSetLayoutBinding ubo_bindings[descriptors_len] = {};
	for(uint8_t i = 0; i < descriptors_len; i++)
	{
		ubo_bindings[i].binding         = i;
		ubo_bindings[i].descriptorType  = descriptor_infos[i].type;
		ubo_bindings[i].descriptorCount = 1;
		ubo_bindings[i].stageFlags      = VK_SHADER_STAGE_VERTEX_BIT;
	}

	VkDescriptorSetLayoutCreateInfo layout_info = {};
	layout_info.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layout_info.bindingCount = descriptors_len;
	layout_info.pBindings    = ubo_bindings;
	VK_VERIFY(vkCreateDescriptorSetLayout(vk->device, &layout_info, 0, &resources->descriptor_layout));

	VkDescriptorPoolSize pool_sizes[descriptors_len] = {};
	for(uint8_t i = 0; i < descriptors_len; i++)
	{
		pool_sizes[i].type            = descriptor_infos[i].type;
		pool_sizes[i].descriptorCount = 1;
	}

	VkDescriptorPoolCreateInfo pool_info = {};
	pool_info.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	pool_info.poolSizeCount = descriptors_len;
	pool_info.pPoolSizes    = pool_sizes;
	pool_info.maxSets       = 1;
	VK_VERIFY(vkCreateDescriptorPool(vk->device, &pool_info, 0, &resources->descriptor_pool));

	VkDescriptorSetAllocateInfo alloc_info = {};
	alloc_info.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	alloc_info.descriptorPool     = resources->descriptor_pool;
	alloc_info.descriptorSetCount = 1;
	alloc_info.pSetLayouts        = &resources->descriptor_layout;
	VK_VERIFY(vkAllocateDescriptorSets(vk->device, &alloc_info, &resources->descriptor_set));

	VkDescriptorBufferInfo buf_infos[descriptors_len] = {};
	VkWriteDescriptorSet write_descriptors[descriptors_len] = {};
	for(uint8_t i = 0; i < descriptors_len; i++)
	{
		buf_infos[i].buffer = vk->host_visible_buffer;
		buf_infos[i].offset = descriptor_infos[i].offset_in_buffer;
		buf_infos[i].range  = descriptor_infos[i].range_in_buffer;

		write_descriptors[i].sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write_descriptors[i].dstSet          = resources->descriptor_set;
		write_descriptors[i].dstBinding      = i;
		write_descriptors[i].dstArrayElement = 0;
		write_descriptors[i].descriptorType  = descriptor_infos[i].type;
		write_descriptors[i].descriptorCount = 1;
		write_descriptors[i].pBufferInfo     = &buf_infos[i];
	}

	vkUpdateDescriptorSets(vk->device, descriptors_len, write_descriptors, 0, 0);

	// Shaders
	uint8_t shader_infos_len = 2;
	VkPipelineShaderStageCreateInfo shader_infos[shader_infos_len] = {};

	shader_infos[0].sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shader_infos[0].stage  = VK_SHADER_STAGE_VERTEX_BIT;
	shader_infos[0].module = shader_vert;
	shader_infos[0].pName  = "main";

	shader_infos[1].sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shader_infos[1].stage  = VK_SHADER_STAGE_FRAGMENT_BIT;
	shader_infos[1].module = shader_frag;
	shader_infos[1].pName  = "main";

	uint8_t bind_descriptions_len = 1;
	VkVertexInputBindingDescription bind_descriptions[bind_descriptions_len] = {};
	bind_descriptions[0].binding   = 0;
	bind_descriptions[0].stride    = vertex_stride;
	bind_descriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	VkVertexInputAttributeDescription attr_descriptions[attribute_descriptions_len] = {};

	for(uint8_t i = 0; i < attribute_descriptions_len; i++)
	{
		attr_descriptions[i].binding  = 0;
		attr_descriptions[i].location = i;
		attr_descriptions[i].format   = attribute_descriptions[i].format;
		attr_descriptions[i].offset   = attribute_descriptions[i].offset;
	}

	VkPipelineVertexInputStateCreateInfo vert_input_info = {};
	vert_input_info.sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vert_input_info.vertexBindingDescriptionCount   = bind_descriptions_len;
	vert_input_info.pVertexBindingDescriptions      = bind_descriptions;
	vert_input_info.vertexAttributeDescriptionCount = attribute_descriptions_len;
	vert_input_info.pVertexAttributeDescriptions    = attr_descriptions;

	VkPipelineInputAssemblyStateCreateInfo input_assembly_info = {};
	input_assembly_info.sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	input_assembly_info.topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	input_assembly_info.primitiveRestartEnable = VK_FALSE;

	// VOLATILE - Length must match dynamic_info.dynamicStateCount.
	VkDynamicState dyn_states[2] = 
	{
		VK_DYNAMIC_STATE_VIEWPORT, 
		VK_DYNAMIC_STATE_SCISSOR 
	};
	VkPipelineDynamicStateCreateInfo dynamic_info = {};
	dynamic_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamic_info.dynamicStateCount = 2;
	dynamic_info.pDynamicStates = dyn_states;

	VkViewport viewport = {};
	viewport.x        = 0.0f;
	viewport.y        = 0.0f;
	viewport.width    = (float)vk->swap_extent.width;
	viewport.width    = (float)vk->swap_extent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor = {};
	scissor.offset = (VkOffset2D){0, 0};
	scissor.extent = vk->swap_extent;

	VkPipelineViewportStateCreateInfo viewport_info = {};
	viewport_info.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewport_info.viewportCount = 1;
	viewport_info.pViewports    = &viewport;
	viewport_info.scissorCount  = 1;
	viewport_info.pScissors     = &scissor;

	VkPipelineRasterizationStateCreateInfo raster_info = {};
	raster_info.sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	raster_info.depthClampEnable        = VK_FALSE;
	raster_info.rasterizerDiscardEnable = VK_FALSE;
	raster_info.polygonMode             = VK_POLYGON_MODE_FILL;
	raster_info.lineWidth               = 1.0f;
	raster_info.cullMode                = VK_CULL_MODE_NONE;
	raster_info.frontFace               = VK_FRONT_FACE_CLOCKWISE;
	raster_info.depthBiasEnable         = VK_FALSE;
	raster_info.depthBiasConstantFactor = 0.0f;
	raster_info.depthBiasClamp          = 0.0f;
	raster_info.depthBiasSlopeFactor    = 0.0f;

	VkPipelineMultisampleStateCreateInfo multisample_info = {};
	multisample_info.sType                 = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisample_info.sampleShadingEnable   = VK_FALSE;
	multisample_info.rasterizationSamples  = vk->render_samples;

	VkPipelineColorBlendAttachmentState color_blend_attachment = {};
	color_blend_attachment.colorWriteMask = 
		VK_COLOR_COMPONENT_R_BIT | 
		VK_COLOR_COMPONENT_G_BIT | 
		VK_COLOR_COMPONENT_B_BIT | 
		VK_COLOR_COMPONENT_A_BIT;
	color_blend_attachment.blendEnable    = VK_FALSE;

	VkPipelineColorBlendStateCreateInfo color_blend_info = {};
	color_blend_info.sType             = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	color_blend_info.logicOpEnable     = VK_FALSE;
	color_blend_info.attachmentCount   = 1;
	color_blend_info.pAttachments      = &color_blend_attachment;
	color_blend_info.blendConstants[0] = 0.0f;
	color_blend_info.blendConstants[1] = 0.0f;
	color_blend_info.blendConstants[2] = 0.0f;
	color_blend_info.blendConstants[3] = 0.0f;

	VkPipelineDepthStencilStateCreateInfo depth_stencil_info = {};
	depth_stencil_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depth_stencil_info.depthTestEnable = VK_TRUE;
	depth_stencil_info.depthWriteEnable = VK_TRUE;
	depth_stencil_info.depthCompareOp = VK_COMPARE_OP_LESS;
	depth_stencil_info.depthBoundsTestEnable = VK_FALSE;
	depth_stencil_info.stencilTestEnable = VK_FALSE;

	VkPipelineLayoutCreateInfo pipeline_layout_info = {};
	pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipeline_layout_info.setLayoutCount = 1;
	pipeline_layout_info.pSetLayouts = &resources->descriptor_layout;
	VK_VERIFY(vkCreatePipelineLayout(vk->device, &pipeline_layout_info, 0, &resources->pipeline_layout));

	VkPipelineRenderingCreateInfoKHR pipeline_render_info = {};
	pipeline_render_info.sType                   = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR; 
	pipeline_render_info.pNext                   = VK_NULL_HANDLE; 
	pipeline_render_info.colorAttachmentCount    = 1; 
	pipeline_render_info.pColorAttachmentFormats = &vk->surface_format.format;
	pipeline_render_info.depthAttachmentFormat   = DEPTH_ATTACHMENT_FORMAT;
	pipeline_render_info.stencilAttachmentFormat = 0;

	VkGraphicsPipelineCreateInfo pipeline_info = {};
	pipeline_info.sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipeline_info.pNext               = &pipeline_render_info;
	pipeline_info.renderPass          = VK_NULL_HANDLE;
	pipeline_info.pInputAssemblyState = &input_assembly_info;
	pipeline_info.pRasterizationState = &raster_info;
	pipeline_info.pColorBlendState    = &color_blend_info;
	pipeline_info.pMultisampleState   = &multisample_info;
	pipeline_info.pViewportState      = &viewport_info;
	pipeline_info.pDepthStencilState  = &depth_stencil_info;
	pipeline_info.pDynamicState       = &dynamic_info;
	pipeline_info.pVertexInputState   = &vert_input_info;
	pipeline_info.stageCount          = shader_infos_len;
	pipeline_info.pStages             = shader_infos;
	pipeline_info.layout              = resources->pipeline_layout;
	VK_VERIFY(vkCreateGraphicsPipelines(vk->device, VK_NULL_HANDLE, 1, &pipeline_info, 0, &resources->pipeline));

	vkDestroyShaderModule(vk->device, shader_vert, 0);
	vkDestroyShaderModule(vk->device, shader_frag, 0);
}
