#pragma once

#define GLFW_INCLUDE_VULKAN // Informs glfw to automatically include vulkan headers
#include <GLFW/glfw3.h>
#include <vector>

struct SwapChainSupportDetails
{
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};