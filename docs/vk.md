# Vulkan usage
Establishing practices to modify or extend the functionality of the current engine.
## Creating an addition to the UBO
At the moment, there is a single UBO with a few different bindings. If we need more data in there, we add a new type to the descriptor_types array found under the heading "Create descriptor set layout, pool, and sets."
