#pragma once


#include <vector>
#include <cstdlib>

#define GLFW_INCLUDE_VULKAN // Informs glfw to automatically include vulkan headers
#include <GLFW/glfw3.h>

class VulkanApplication
{
public:
	void run();

private:
	/** GLFW **/
	const uint32_t WIDTH = 800;
	const uint32_t HEIGHT = 600;
	GLFWwindow* window;

	void initWindow();
	void cleanupWindow();

	/** VULKAN **/ 
	VkInstance instance;

	void initVulkan();
	void cleanupVulkan();
	void createInstance();
	bool checkGLFWExtensionSupport(const char** glfwExtensions, uint32_t glfwExtensionCount);

	void mainLoop();
	void cleanup();

};