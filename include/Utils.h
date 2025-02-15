#ifndef __UTILS_H__
#define __UTILS_H__

#include <string>
#include <cassert>
#include <cstdint>
#include <chrono>
#include <thread>


static std::vector<char> ReadFile(const std::string& filename)
{
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open())
    {
        assert(false);
        throw std::runtime_error("failed to open file!");
    }

    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();

    return buffer;
}

void LimitFPS(const int targetFps = 30)
{
    const int frameTimeMillisec = 1000 / targetFps;

    static auto lastTime = std::chrono::high_resolution_clock::now();

    auto currentTime = std::chrono::high_resolution_clock::now();
    auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastTime).count();

    if (elapsedTime < frameTimeMillisec)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(frameTimeMillisec - elapsedTime));
    }

    lastTime = std::chrono::high_resolution_clock::now();
}

#endif // __UTILS_H__
