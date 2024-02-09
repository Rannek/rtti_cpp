/******************************************************************************
 * Author: Rannek
 * Date: 2024-02-08
 * 
 * Description:
 * A utility to process binary files for extracting and saving RTTI images. 
 * It searches for specific image headers, reads image dimensions, and extracts
 * images based on the provided dimensions. Images are then converted to RGB
 * format and saved. Supports images up to 2000x2000 in size.
 *
 * Usage:
 * ./executable <file_path>
 ******************************************************************************/

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>
#include <stdexcept>

namespace fs = std::filesystem;

// Configurable parameters
const std::vector<std::string> Headers = {"Image8"};
const int Max_Image_Width = 2000;
const int Max_Image_Height = 2000;

// Function declarations
bool FindImageHeader(std::ifstream& file);
std::pair<int, int> ReadDimensions(std::ifstream& file);
void SaveAsBMP(const std::string& output_path, std::vector<unsigned char>& img_data, int width, int height);
void ProcessFile(const std::string& file_path);

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <file_path>\n";
        return 1;
    }

    ProcessFile(argv[1]);
    return 0;
}

bool FindImageHeader(std::ifstream& file) {
    std::string buffer;
    char ch;

    while (file.get(ch)) {
        buffer.push_back(ch);

        // Check each header in Headers
        for (const auto& header : Headers) {
            if (buffer.size() > header.size()) {
                buffer.erase(buffer.begin());
            }

            if (buffer == header) {
                return true;
            }
        }

        // Reset buffer if it's longer than the longest header
        if (buffer.size() > Headers[0].length()) {
            buffer.erase(buffer.begin());
        }
    }
    return false;
}

std::pair<int, int> ReadDimensions(std::ifstream& file) {
    unsigned char bytes[4];
    int width, height;

    // Reading width
    if (!file.read(reinterpret_cast<char*>(bytes), sizeof(bytes))) {
        throw std::runtime_error("Failed to read width from file.");
    }
    width = bytes[0] | (bytes[1] << 8) | (bytes[2] << 16) | (bytes[3] << 24);

    // Reading height
    if (!file.read(reinterpret_cast<char*>(bytes), sizeof(bytes))) {
        throw std::runtime_error("Failed to read height from file.");
    }
    height = bytes[0] | (bytes[1] << 8) | (bytes[2] << 16) | (bytes[3] << 24);

    return {width, height};
}

void SaveAsBMP(const std::string& output_path, std::vector<unsigned char>& img_data, int width, int height) {
    std::ofstream file(output_path, std::ios::binary);
    if (!file) {
        std::cerr << "Failed to open output file.\n";
        return;
    }

    // BMP File Header
    unsigned char bmpFileHeader[14] = {'B', 'M', 0, 0, 0, 0, 0, 0, 0, 0, 54, 0, 0, 0};
    // BMP Info Header
    unsigned char bmpInfoHeader[40] = {40, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 24, 0};

    // Calculate padding for alignment to 4 bytes
    int paddingAmount = (4 - (width * 3) % 4) % 4;
    int fileSize = 54 + (width * 3 + paddingAmount) * height;

    // Update file size in header
    bmpFileHeader[2] = fileSize;
    bmpFileHeader[3] = fileSize >> 8;
    bmpFileHeader[4] = fileSize >> 16;
    bmpFileHeader[5] = fileSize >> 24;

    // Width and height in info header
    bmpInfoHeader[4] = width;
    bmpInfoHeader[5] = width >> 8;
    bmpInfoHeader[6] = width >> 16;
    bmpInfoHeader[7] = width >> 24;
    bmpInfoHeader[8] = height;
    bmpInfoHeader[9] = height >> 8;
    bmpInfoHeader[10] = height >> 16;
    bmpInfoHeader[11] = height >> 24;

    // Write the headers
    file.write(reinterpret_cast<char*>(bmpFileHeader), sizeof(bmpFileHeader));
    file.write(reinterpret_cast<char*>(bmpInfoHeader), sizeof(bmpInfoHeader));

    // Write image data with padding and channel swapping
    unsigned char padding[3] = {0, 0, 0};
    for (int i = height - 1; i >= 0; i--) {
        for (int j = 0; j < width; j++) {
            // Swap the red and blue components for each pixel
            std::swap(img_data[(i * width * 3) + (j * 3)], img_data[(i * width * 3) + (j * 3) + 2]);
        }
        file.write(reinterpret_cast<char*>(img_data.data() + (i * width * 3)), width * 3);
        file.write(reinterpret_cast<char*>(padding), paddingAmount);
    }
}

void ProcessFile(const std::string& file_path) {
    std::ifstream file(file_path, std::ios::binary);
    if (!file) {
        return;
    }

    while (FindImageHeader(file)) {
        file.ignore(1); // Skip newline character

        try {
            auto [width, height] = ReadDimensions(file);

            // Only process if dimensions are within limits
            if (width <= Max_Image_Width && height <= Max_Image_Height) {
                std::vector<unsigned char> img_data(width * height * 3);
                if (!file.read(reinterpret_cast<char*>(img_data.data()), img_data.size())) {
                    continue;
                }

                static int image_counter = 0;
                std::string output_path = fs::path(file_path).stem().string() + "_extracted_" + std::to_string(++image_counter) + ".bmp";
                SaveAsBMP(output_path, img_data, width, height);
            } else {
                // Skip this image due to size constraints
                continue;
            }

        } catch (const std::runtime_error& e) {
            continue;
        }
    }
}
