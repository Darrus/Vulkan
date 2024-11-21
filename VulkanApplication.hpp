#pragma once


#include <cstdlib>

#define GLFW_INCLUDE_VULKAN // Informs glfw to automatically include vulkan headers
#include <GLFW/glfw3.h>

class VulkanApplication
{
public:
	void run();

private:
	const uint32_t WIDTH = 800;
	const uint32_t HEIGHT = 600;
	GLFWwindow* window;

	void initWindow();
	void initVulkan();
	void mainLoop();
	void cleanup();
};