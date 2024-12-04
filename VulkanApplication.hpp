#pragma once

#include <vector>
#include <cstdint>

#define GLFW_INCLUDE_VULKAN // Informs glfw to automatically include vulkan headers
#include <GLFW/glfw3.h>
#include "QueueFamilyIndices.hpp"
#include "SwapChainSupportDetails.hpp"

class VulkanApplication
{
public:
	void run();

private:
	void mainLoop();
	void cleanup();

	/** GLFW **/
	const uint32_t WIDTH = 800;
	const uint32_t HEIGHT = 600;
	GLFWwindow *window;

	void initGLFW();
	void cleanupGLFW();

	/** VULKAN **/
	const std::vector<const char *> deviceExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};
	const std::vector<const char *> validationLayers = {
		"VK_LAYER_KHRONOS_validation" // All useful standard validation bundled into this layer
	};

	VkInstance instance;
	VkDebugUtilsMessengerEXT debugMessenger;
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	VkDevice device; // Logical device, interfaces to physical device
	VkQueue graphicsQueue;
	VkSurfaceKHR surface;
	VkQueue presentQueue;
	VkSwapchainKHR swapChain;
	std::vector<VkImage> swapChainImages;
	VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent;

#ifndef NDEBUG // C++ Standard Macro, stands for "not debug"
	const bool enableValidationLayers = false;
#else
	const bool enableValidationLayers = true;
#endif

	void initVulkan();
	void cleanupVulkan();
	void createInstance();

	/* EXTENSIONS */
	std::vector<const char *> getRequiredInstanceExtensions();
	bool checkGLFWExtensionSupport(const char **glfwExtensions, uint32_t glfwExtensionCount);
	bool checkValidationLayerSupport();

	/* DEBUG MESSENGER */
	void setupDebugMessenger();
	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo);
	VkResult createDebugUtilsMessengerEXT(
		VkInstance instance,
		const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
		const VkAllocationCallbacks *pAllocator,
		VkDebugUtilsMessengerEXT *pDebugMessenger);
	void destroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks *pAllocator);

	// VKAPI_ATTR and VKAPI_CALL are used to call platform specific macros
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
		void *pUserData);

	/* SURFACE */
	void createSurface();

	/* PHYSICAL DEVICE */
	void pickPhysicalDevice();
	int rateDeviceSuitability(VkPhysicalDevice device);
	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
	bool checkDeviceExtensionSupport(VkPhysicalDevice device);

	/* LOGICAL DEVICE */
	void createLogicalDevice();

	/* SWAP CHAIN */
	void createSwapChain();
	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availableModes);
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
};