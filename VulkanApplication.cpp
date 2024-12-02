#include "VulkanApplication.hpp"

#include <iostream>
#include <string>
#include <stdexcept>
#include <unordered_set>

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
}

void VulkanApplication::cleanupVulkan()
{
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

	// Get the extensions required and pass to createInfo
	std::vector<const char*> extensions = getRequiredExtensions();
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

std::vector<const char*> VulkanApplication::getRequiredExtensions()
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
	std::unordered_set<std::string> extensionSet;
	for (const VkExtensionProperties& extension : extensions)
	{
		extensionSet.insert(extension.extensionName);
	}

	// Loop through all glfw extenions and check if supported
	for (int i = 0; i < glfwExtensionCount; ++i)
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
	std::unordered_set<std::string> layerSet;
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