#include "VulkanApplication.hpp"

#include <stdexcept>

void VulkanApplication::run()
{
	initWindow();
	initVulkan();
	mainLoop();
	cleanup();
}

void VulkanApplication::initWindow()
{
	glfwInit(); // intializes GLFW Library

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // GLFW by default initializes with OpenGL context, this informs it not to.
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); // Disable resizable window

	window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr); // Create window
}

void VulkanApplication::cleanupWindow()
{
	glfwDestroyWindow(window);
	glfwTerminate();
}

void VulkanApplication::initVulkan()
{
	createInstance();
}

void VulkanApplication::cleanupVulkan()
{
	vkDestroyInstance(instance, nullptr);
}

void VulkanApplication::createInstance()
{
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

	// Vulkan is platform agnostic API
	// As such we need a extension to interface with the window system
	// GLFW is able to provide what extensions are required 
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	// Get the extensions required and pass to createInfo
	createInfo.enabledExtensionCount = glfwExtensionCount;
	createInfo.ppEnabledExtensionNames = glfwExtensions;

	createInfo.enabledLayerCount = 0;

	if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create vulkan instance!");
	}
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
	cleanupWindow();
}
