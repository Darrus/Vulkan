#include <iostream>
#include <stdexcept>

#include "VulkanApplication.hpp"

int main(int argc, char** argv) {
    VulkanApplication app;

    try {
        app.run();
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}