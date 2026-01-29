#pragma once

#include <array>
#include <Debug.h>
#include <ImageFileLoader.h>
#include <string>

#ifdef _WIN32
    #define POPEN _popen
    #define PCLOSE _pclose
#else
    #include <stdio.h>
    #define POPEN popen
    #define PCLOSE pclose
#endif

static int executeCommandWithLogs(const std::string& command)
{
    Wolf::Debug::sendInfo("Executing command: " + command);

    std::array<char, 1024> buffer;
    std::string result;
    FILE* pipe = POPEN(command.c_str(), "r");
    if (!pipe)
        Wolf::Debug::sendError("Can't execute command");

    try
    {
        while (fgets(buffer.data(), buffer.size(), pipe) != nullptr)
        {
            result += buffer.data();
        }
    }
    catch (const std::exception& e)
    {
        Wolf::Debug::sendError("Error while getting console result for command: " + command);
    }

    Wolf::Debug::sendInfo("Command result: " + result);

    int returnCode = PCLOSE(pipe);
    Wolf::Debug::sendInfo("Command retuned: " + std::to_string(returnCode));

    return returnCode;
}

static constexpr float MAX_TOLERANCE = 1.0f;
static float compareImages(const std::string& image1, const std::string& image2)
{
    Wolf::ImageFileLoader imageFileLoader1(image1);
    Wolf::ImageFileLoader imageFileLoader2(image2);

    if (imageFileLoader1.getWidth() != imageFileLoader2.getWidth() || imageFileLoader1.getHeight() != imageFileLoader2.getHeight() || imageFileLoader1.getChannelCount() != imageFileLoader2.getChannelCount())
        return MAX_TOLERANCE;

    if (imageFileLoader1.getFormat() != Wolf::Format::R8G8B8A8_UNORM)
    {
        Wolf::Debug::sendError("Unsupported image format");
        return MAX_TOLERANCE;
    }

    const Wolf::ImageCompression::RGBA8* pixels1 = reinterpret_cast<const Wolf::ImageCompression::RGBA8*>(imageFileLoader1.getPixels());
    const Wolf::ImageCompression::RGBA8* pixels2 = reinterpret_cast<const Wolf::ImageCompression::RGBA8*>(imageFileLoader2.getPixels());

    const uint32_t pixelCount = imageFileLoader1.getWidth() * imageFileLoader1.getHeight();
    float totalDiff = 0.0f;
    for (uint32_t pixelIdx = 0; pixelIdx < pixelCount; pixelIdx++)
    {
        const Wolf::ImageCompression::RGBA8& pixel1 = pixels1[pixelIdx];
        const Wolf::ImageCompression::RGBA8& pixel2 = pixels2[pixelIdx];

        float pixelDiff = 0.0f;
        pixelDiff += glm::pow((pixel1.r - pixel2.r) / 255.0f, 2.0f);
        pixelDiff += glm::pow((pixel1.g - pixel2.g) / 255.0f, 2.0f);
        pixelDiff += glm::pow((pixel1.b - pixel2.b) / 255.0f, 2.0f);
        pixelDiff /= 3.0f;

        totalDiff += pixelDiff;
    }

    return totalDiff / static_cast<float>(pixelCount);
}