#include "VulkanApplication.hpp"

#include <iostream>
#include <string>
#include <stdexcept>
#include <map>
#include <set>
#include <algorithm>

/*
* You'll see a lot of variable or functions that ends with KHR
* KHR are extensions that were approved by KHRonos 
*/ 

void VulkanApplication::run()
{
	initGLFW();
	initVulkan();
	mainLoop();
	cleanup();
}

void VulkanApplication::mainLoop()
{
	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
	}
}

void VulkanApplication::cleanup()
{
	cleanupVulkan();
	cleanupGLFW();
}


void VulkanApplication::initGLFW()
{
	glfwInit(); // intializes GLFW Library

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // GLFW by default initializes with OpenGL context, this informs it not to.
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); // Disable resizable window

	window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr); // Create window
}

void VulkanApplication::cleanupGLFW()
{
	glfwDestroyWindow(window);
	glfwTerminate();
}

void VulkanApplication::initVulkan()
{
	createInstance();
	setupDebugMessenger();
	createSurface();
	pickPhysicalDevice();
	createLogicalDevice();
	createSwapChain();
	createImageViews();
}

void VulkanApplication::cleanupVulkan()
{
	for (auto imageView : swapChainImageViews) {
		vkDestroyImageView(device, imageView, nullptr);
	}

	vkDestroySwapchainKHR(device, swapChain, nullptr);
	vkDestroySurfaceKHR(instance, surface, nullptr);
	vkDestroyDevice(device, nullptr);

	if (enableValidationLayers) {
		destroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
	}

	vkDestroyInstance(instance, nullptr);
}

void VulkanApplication::createInstance()
{
	if (enableValidationLayers && !checkValidationLayerSupport()) {
		throw std::runtime_error("Validation layers requested, but not available!");
	}

	// {} is to value initialize the struct
	// VkApplicationInfo is optional but good to have
	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO; // Defines what type of struct is it
	appInfo.pApplicationName = "Vulkan";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "No Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;


	VkInstanceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	// Get the required instance extensions and pass to createInfo
	std::vector<const char*> extensions = getRequiredInstanceExtensions();
	createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	createInfo.ppEnabledExtensionNames = extensions.data();

	if (enableValidationLayers) 
	{
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();

		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
		populateDebugMessengerCreateInfo(debugCreateInfo);
		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
	}
	else
	{
		createInfo.enabledLayerCount = 0;
		createInfo.pNext = nullptr;
	}

	if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create vulkan instance!");
	}
}

std::vector<const char*> VulkanApplication::getRequiredInstanceExtensions()
{
	// Vulkan is platform agnostic API
	// As such we need a extension to interface with the window system
	// GLFW is able to provide what extensions are required
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

	if (enableValidationLayers)
	{
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	if (!checkGLFWExtensionSupport(glfwExtensions, glfwExtensionCount)) {
		throw std::runtime_error("Not all GLFW extensions are supported by Vulkan!");
	}

	return extensions;
}

bool VulkanApplication::checkGLFWExtensionSupport(const char** glfwExtensions, uint32_t glfwExtensionCount)
{
	uint32_t extensionCount;
	// First get the amount of supported extensions
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
	std::vector<VkExtensionProperties> extensions(extensionCount); // Dynamically allocate size based on count of extensions
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data()); // Fill the vector with details about the extensions

	// Store into a unique hash set
	// Use std::string so that we can do a value comparison instead of address comparison
	std::set<std::string> extensionSet;
	for (const VkExtensionProperties& extension : extensions)
	{
		extensionSet.insert(extension.extensionName);
	}

	// Loop through all glfw extenions and check if supported
	for (unsigned int i = 0; i < glfwExtensionCount; ++i)
	{
		std::string glfwExtension(glfwExtensions[i]);
		if (extensionSet.find(glfwExtension) == extensionSet.end()) {
			return false;
		}
	}

	return true;
}

bool VulkanApplication::checkValidationLayerSupport()
{
	// First get the amount of supported layers
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount); // Dynamically allocate size based on count of layers
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data()); // Fill the vector with details about the layers

	// Store into a unique hash set
	// Use std::string so that we can do a value comparison instead of address comparison
	std::set<std::string> layerSet;
	for (const VkLayerProperties& layerProperties : availableLayers)
	{
		layerSet.insert(layerProperties.layerName);
	}

	// Loop through intended validation layers and check if supported
	for (std::string layerName : validationLayers) {
		if (layerSet.find(layerName) == layerSet.end()) {
			return false;
		}
	}

	return true;
}

void VulkanApplication::setupDebugMessenger()
{
	if (!enableValidationLayers) return;

	VkDebugUtilsMessengerCreateInfoEXT createInfo;
	populateDebugMessengerCreateInfo(createInfo);

	if (createDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
		throw std::runtime_error("failed to setup debug messenger!");
	}
}

/**
* Message Severity
* - VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
* - VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT
* - VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
* - VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT
*
* Message Type
* - VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT, Event has happened that is unrelated to the specification or performance
* - VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT, Something has happened that violates the specification or indicates a possible mistake
* - VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT, Potential non-optimal use of Vulkan
**/
void VulkanApplication::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
	createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = debugCallback;
	createInfo.pUserData = nullptr;
}

VkResult VulkanApplication::createDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
{
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr) {
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

void VulkanApplication::destroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
{
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr) {
		func(instance, debugMessenger, pAllocator);
	}
}

/**
* pCallbackData
* - pMessage 
* - pObjects
* - objectCount
**/
VkBool32 VulkanApplication::debugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, 
	VkDebugUtilsMessageTypeFlagsEXT messageType, 
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, 
	void* pUserData)
{
	std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

	return VK_FALSE;
}

void VulkanApplication::createSurface()
{
	if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
		throw std::runtime_error("failed to create window surface!");
	}
}

void VulkanApplication::pickPhysicalDevice()
{
	uint32_t deviceCount = 0;
	// First grab the count of physical devices
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

	if (deviceCount == 0) {
		throw std::runtime_error("failed to find GPUs with Vulkan support!");
	}

	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());


	// Sorted map to automatically sort candidates on insert
	std::multimap<int, VkPhysicalDevice> candidates;

	for (const auto& device : devices) {
		int score = rateDeviceSuitability(device);
		candidates.insert(std::pair<int, VkPhysicalDevice>(score, device));
	}

	// Checks if the score is more than 0
	if (candidates.rbegin()->first > 0) {
		physicalDevice = candidates.rbegin()->second; // Assign the device in the map
	} else {
		throw std::runtime_error("failed to find a suitable GPU!");
	}
}

int VulkanApplication::rateDeviceSuitability(VkPhysicalDevice device)
{
	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(device, &deviceProperties);

	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

	int score = 0;

	// Discrete GPU are usually more performant
	if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
		score += 1000;
	}

	// Maximum possible size of textures affects graphics quality
	score += deviceProperties.limits.maxImageDimension2D;

	// Application requires geometryShader
	if (!deviceFeatures.geometryShader) {
		return 0;
	}

	// Check if device has extensions required supported
	if (!checkDeviceExtensionSupport(device)) {
		return 0;
	}

	// Check if device supports swap chain
	SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
	if (swapChainSupport.formats.empty() || swapChainSupport.presentModes.empty()) {
		return 0;
	}

	// Check device has all queue families supported
	QueueFamilyIndices indices = findQueueFamilies(device);
	if (!indices.isComplete()) {
		return 0;
	}

	return score;
}

QueueFamilyIndices VulkanApplication::findQueueFamilies(VkPhysicalDevice device)
{
	QueueFamilyIndices indices;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());
	 
	for (unsigned int i = 0; i < queueFamilyCount; ++i) {
		if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			indices.graphicsFamily = i;
		}

		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

		if (presentSupport) {
			indices.presentFamily = i;
		}

		if (indices.isComplete()) {
			break;
		}
	}

	return indices;
}

bool VulkanApplication::checkDeviceExtensionSupport(VkPhysicalDevice device)
{
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

	std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

	for (const auto& extension : availableExtensions) {
		requiredExtensions.erase(extension.extensionName);
	}

	return requiredExtensions.empty();
}

void VulkanApplication::createLogicalDevice()
{
	QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

	float queuePriority = 1.0f;
	for (uint32_t queueFamily : uniqueQueueFamilies) {
		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(queueCreateInfo);
	}
	
	VkPhysicalDeviceFeatures deviceFeatures{};
	vkGetPhysicalDeviceFeatures(physicalDevice, &deviceFeatures);

	VkDeviceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

	// Set queues
	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	createInfo.pQueueCreateInfos = queueCreateInfos.data();

	// Set enabled device features
	createInfo.pEnabledFeatures = &deviceFeatures;

	// Set enabled extensions
	createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
	createInfo.ppEnabledExtensionNames = deviceExtensions.data();

	// Set validation layers
	if (enableValidationLayers) {
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
	}
	else 
	{
		createInfo.enabledLayerCount = 0;
	}

	// Notice how it's quite similar to instantiating instance
	// Only difference is that these features are device specific now
	if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
		throw std::runtime_error("failed to create logical device!");
	}

	vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
	vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
}

void VulkanApplication::createSwapChain()
{
	SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);
	VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
	VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
	VkExtent2D swapExtent = chooseSwapExtent(swapChainSupport.capabilities);
	
	// Minimum images in swap chain
	// +1 because we don't want to wait for the driver to complete it's operations before being allowed to query a new image hence causing a delay
	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

	// Ensure that the image count above does not exceed the max image count
	// 0 is a special value that indicates there's no max image count
	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
		imageCount = swapChainSupport.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = surface;
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = swapExtent;

	// Specifies the amount of layers an image has
	// Usually defaults to 1, unless developing a stereoscopic 3D application
	createInfo.imageArrayLayers = 1;

	// Specifies what kind of operations the image is used for
	// VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT means we're going to render directly to them
	// VK_IMAGE_USAGE_TRANS_DST_BIT can be used for post-processing an image and use a memory operation to transfer the rendered image to a swap chain image
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
	uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

	// Image Sharing Mode indicates how an image is shared between different queues
	// VK_SHARING_MODE_CONCURRENT, Images can be used across multiple queue families without explicit ownership transfers
	// VK_SHARING_MODE_EXCLUSIVE, An image is owned by one queue family at a time and ownership must be explicitly transferred before using it in another queue family.
	// This option offers the best performance.
	if (indices.graphicsFamily != indices.presentFamily) {
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	} else {
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0;
		createInfo.pQueueFamilyIndices = nullptr;
	}

	// Allows us to apply transformation on the images in the swap chain
	// currentTransform indicates to not perform any transformations
	createInfo.preTransform = swapChainSupport.capabilities.currentTransform;

	// Specifies if the alpha channel should be used for blending with other windows in the window system
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

	// When clipped is set to VK_TRUE it indicates that we want to ignore pixels that are obscured
	// For example, another window covering our window
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;

	// It's possible for swap chains to become invalid, for example window resizing
	// The swap chain would then need to be recreated and referenced from the old swap chain
	// Ignore for now
	createInfo.oldSwapchain = VK_NULL_HANDLE;

	if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
		throw std::runtime_error("failed to create swap chain!");
	}

	vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
	swapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());

	swapChainImageFormat = createInfo.imageFormat;
	swapChainExtent = createInfo.imageExtent;
}

SwapChainSupportDetails VulkanApplication::querySwapChainSupport(VkPhysicalDevice device)
{
	SwapChainSupportDetails details{};

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

	if (formatCount > 0) {
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
	}

	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

	if (presentModeCount > 0) {
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
	}

	return details;
}

VkSurfaceFormatKHR VulkanApplication::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
	for (const auto& surfaceFormat : availableFormats) {
		// - Format indicates the way the color is stored, Blue Green Red Alpha in order, and the 8 represents the amount of bits per channel adding up to a total of 32 bits pixel
		// Reason why we're using BGRA is due to backwards compatability as older hardware used to store colors in BGRA and not RGBA
		// - Color Space checks if sRGB is supported
		if (surfaceFormat.format == VK_FORMAT_B8G8R8A8_SRGB && surfaceFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return surfaceFormat;
		}
	}

	return availableFormats[0];
}

/** PRESENT MODES

* VK_PRESENT_MODE_IMMEDIATE_KHR, Images submitted by your application are transferred to the screen right away, which may result in tearing.

* VK_PRESENT_MODE_FIFO_KHR, The swap chain is a queue where the display takes an image from the front of the queue when the display is refreshed 
and the program inserts rendered images at the back of the queue. If the queue is full then the program has to wait. This is most similar to vertical 
sync as found in modern games. The moment that the display is refreshed is known as "vertical blank".

* VK_PRESENT_MODE_FIFO_RELAXED_KHR, This mode only differs from the previous one if the application is late and the queue was empty at the last vertical blank. 
Instead of waiting for the next vertical blank, the image is transferred right away when it finally arrives. This may result in visible tearing.

* VK_PRESENT_MODE_MAILBOX_KHR, This is another variation of the second mode. Instead of blocking the application when the queue is full, 
the images that are already queued are simply replaced with the newer ones. This mode can be used to render frames as fast as possible while 
still avoiding tearing, resulting in fewer latency issues than standard vertical sync. This is commonly known as "triple buffering", although the existence 
of three buffers alone does not necessarily mean that the framerate is unlocked.
*/
VkPresentModeKHR VulkanApplication::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availableModes)
{
	for (const auto& presentMode : availableModes) {
		if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
			return presentMode;
		}
	}

	return VK_PRESENT_MODE_FIFO_KHR;
}

// The Swap Extent is the resolution of the swap chain images in pixels, which usually matches the window's resolution
// Screen Coordinates != Pixel, when we defined GLFW WIDTH and HEIGHT above it's in screen coordinates
VkExtent2D VulkanApplication::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
{
	// Current Extent already has resolution that matches current window
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
		return capabilities.currentExtent;
	} else { // Select best resolution available
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);

		VkExtent2D actualExtent = {
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)
		};
		
		actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
		
		return actualExtent;
	}
}

void VulkanApplication::createImageViews()
{
	swapChainImageViews.resize(swapChainImages.size());

	for (size_t i = 0; i < swapChainImages.size(); ++i) {
		VkImageViewCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = swapChainImages[i];

		// View Type allows you to treat images as 1D, 2D, 3D textures and cube maps
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = swapChainImageFormat;

		// Allows you to swizzle the color channels around
		// VK_COMPONENT_SWIZZILE_IDENTITY is default
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		// Sub-Resource Range describes what th image purpose is and which part of the image should be accessed
		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView(device, &createInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create image views!");
		}
	}
}
