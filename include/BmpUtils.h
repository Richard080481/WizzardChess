#ifndef __BMP_UTILS_H__
#define __BMP_UTILS_H__

#include <cstdint>
#include <string>
#include <vector>
#include <fstream>

#pragma pack(push, 1) // Ensures the BMP structures are byte-aligned

struct BMPFileHeader
{
    uint16_t fileType{0x4D42}; // "BM"
    uint32_t fileSize{0};
    uint16_t reserved1{0};
    uint16_t reserved2{0};
    uint32_t offsetData{54}; // Header size (14 + 40)
};

struct BMPInfoHeader
{
    uint32_t size{40}; // Header size
    int32_t width{0};
    int32_t height{0};
    uint16_t planes{1};
    uint16_t bitCount{8}; // 8-bit grayscale
    uint32_t compression{0};
    uint32_t imageSize{0};
    int32_t xPixelsPerMeter{2835};
    int32_t yPixelsPerMeter{2835};
    uint32_t colorsUsed{256}; // Grayscale palette
    uint32_t colorsImportant{256};
};

struct BMPColorTable
{
    uint8_t r, g, b, a;
};

#pragma pack(pop) // Restore default alignment

void SaveBMP(const std::string& filename, const std::vector<uint8_t>& pixelData, int width, int height, bool isGrayscale)
{
    BMPFileHeader fileHeader;
    BMPInfoHeader infoHeader;

    int bytesPerPixel   = isGrayscale ? 1 : 3;  // 8-bit grayscale or 24-bit RGB
    int rowSize         = ((width * bytesPerPixel + 3) & ~3); // BMP rows must be 4-byte aligned
    int imageSize       = rowSize * height;

    fileHeader.fileSize         = sizeof(BMPFileHeader) + sizeof(BMPInfoHeader) + (isGrayscale ? 256 * 4 : 0) + imageSize;
    infoHeader.width            = width;
    infoHeader.height           = -height;  // BMP uses bottom-up by default, negative to store top-down
    infoHeader.bitCount         = isGrayscale ? 8 : 24;
    infoHeader.imageSize        = imageSize;
    infoHeader.colorsUsed       = isGrayscale ? 256 : 0;
    infoHeader.colorsImportant  = isGrayscale ? 256 : 0;

    std::ofstream file(filename, std::ios::out | std::ios::binary);
    if (!file)
    {
        throw std::runtime_error("Failed to open file for writing BMP");
    }

    // Write headers
    file.write(reinterpret_cast<const char*>(&fileHeader), sizeof(fileHeader));
    file.write(reinterpret_cast<const char*>(&infoHeader), sizeof(infoHeader));

    // Grayscale: Write color palette (256 shades from black to white)
    if (isGrayscale)
    {
        for (int i = 0; i < 256; i++)
        {
            BMPColorTable color = { static_cast<uint8_t>(i), static_cast<uint8_t>(i), static_cast<uint8_t>(i), 0 };
            file.write(reinterpret_cast<const char*>(&color), sizeof(color));
        }
    }

    // Convert BGRA to BMP RGB format
    std::vector<uint8_t> bmpData(imageSize, 0);
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            int dstIndex = y * rowSize + x * bytesPerPixel;

            if (isGrayscale)
            {
                int srcIndex = (y * width + x);
                // For grayscale, assume pixelData is already a single channel
                bmpData[dstIndex] = pixelData[srcIndex]; // Use first channel for grayscale
            }
            else
            {
                int srcIndex = (y * width + x) * 4; // Vulkan BGRA
                // Convert BGRA to BMP's expected BGR format
                bmpData[dstIndex] = pixelData[srcIndex];       // B
                bmpData[dstIndex + 1] = pixelData[srcIndex + 1]; // G
                bmpData[dstIndex + 2] = pixelData[srcIndex + 2]; // R
            }
        }
    }

    // Write pixel data
    file.write(reinterpret_cast<const char*>(bmpData.data()), bmpData.size());
    file.close();
}

#endif // __BMP_UTILS_H__
