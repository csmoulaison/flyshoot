#define MAX_INSTANCES 256

struct vk_ubo_global_world
{
	alignas(16) mat4 view;
	alignas(16) mat4 projection;

	alignas(16) struct v3 clear_color;
	float max_draw_distance_z;
};

// TODO - Our own m4 struct
// TODO - I figure optimally, we probably want to precompute proj * view every
// frame as our uniform buffer, then push the model as a push constant.
struct vk_ubo_global
{
	alignas(64) struct vk_ubo_global_world world;
	alignas(64) struct v2 reticle_pos;
};

struct vk_ubo_instance
{
	alignas(16) mat4 models[MAX_INSTANCES];
};

struct vk_host_memory
{
	alignas(64) struct vk_ubo_global global;
	alignas(64) struct vk_ubo_instance instance;
};

struct vk_pipeline_resources
{
	VkDescriptorSetLayout descriptor_layout;
	VkDescriptorPool      descriptor_pool;
	VkDescriptorSet       descriptor_set;
	VkPipeline            pipeline;
	VkPipelineLayout      pipeline_layout;
};

struct vk_mesh_data
{
	void*    vertex_memory;
	void*    index_memory;

	// In bytes
	uint32_t vertex_stride;

	uint32_t vertices_len;
	uint32_t indices_len;

	uint32_t buffer_offset_vertex;
	uint32_t buffer_offset_index;
};

struct vk_context
{
	VkInstance                   instance;
	VkPhysicalDevice             physical_device;
	VkDevice                     device;
	VkSurfaceKHR                 surface;
	VkSurfaceFormatKHR           surface_format;

	VkQueue                      queue_graphics;
	VkQueue                      queue_present;

	VkImageView                  render_view;
	VkImage                      render_image;
	// TODO - Can this be pulled into device_local_memory, and would that be
	// inadvisable?
	VkDeviceMemory               render_image_memory;
	VkSampleCountFlagBits        render_samples;

	VkImageView                  depth_view;
	VkImage                      depth_image;
	VkDeviceMemory               depth_image_memory;

	VkSwapchainKHR               swapchain;
	VkExtent2D                   swap_extent;
	VkImageView                  swap_views[MAX_SWAP_IMAGES];
	VkImage                      swap_images[MAX_SWAP_IMAGES];
	uint32_t                     swap_images_len;

	struct vk_pipeline_resources pipeline_resources_world;
	struct vk_pipeline_resources pipeline_resources_reticle;

	struct vk_mesh_data          mesh_data_cube;
	struct vk_mesh_data          mesh_data_reticle;

	VkBuffer                     device_local_buffer;
	VkDeviceMemory               device_local_memory;

	VkBuffer                     host_visible_buffer;
	VkDeviceMemory               host_visible_memory;
	void*                        host_visible_mapped;

	VkCommandPool                command_pool;
	VkCommandBuffer              command_buffer;

	VkSemaphore                  semaphore_image_available;
	VkSemaphore                  semaphore_render_finished;

	VkImage                      texture_image;
	VkDeviceMemory               texture_memory;
};

struct vk_platform
{
	VkResult(*create_surface_callback)(struct vk_context* vk, void* context);
	void*   context;
	char**  window_extensions;
	uint8_t window_extensions_len;
};


struct vk_create_swapchain_result
{
	VkSurfaceFormatKHR surface_format;
};

struct vk_cube_vertex
{
	struct v3 pos;
	struct v3 color;
};

struct vk_reticle_vertex
{
	struct v2 pos;
};

struct vk_descriptor_info
{
	VkDescriptorType type;
	VkDeviceSize offset_in_buffer;
	VkDeviceSize range_in_buffer;
};

struct vk_attribute_description
{
	VkFormat format;
	uint32_t offset;
};
