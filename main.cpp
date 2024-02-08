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
#include <opencv2/opencv.hpp>

namespace fs = std::filesystem;

// Function declarations
bool FindImageHeader(std::ifstream& file);
int ReadDimension(std::ifstream& file);
cv::Mat ExtractImage(std::ifstream& file, int width, int height);
void ProcessFile(const std::string& file_path);

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <file_path>\n";
        return 1;
    }

    ProcessFile(argv[1]);
    return 0;
}

// Function definitions

bool FindImageHeader(std::ifstream& file) {
    const std::vector<unsigned char> kTargetHeader = {0x49, 0x6D, 0x61, 0x67, 0x65, 0x38}; // "Image8"
    std::vector<unsigned char> buffer;
    char ch;

    while (file.get(ch)) {
        buffer.push_back(static_cast<unsigned char>(ch));

        if (buffer.size() > kTargetHeader.size()) {
            buffer.erase(buffer.begin());
        }

        if (buffer == kTargetHeader) {
            std::cout << "Found 'Image8' header.\n";
            return true;
        }
    }
    return false;
}

int ReadDimension(std::ifstream& file) {
    unsigned char bytes[4];
    if (!file.read(reinterpret_cast<char*>(bytes), sizeof(bytes))) {
        throw std::runtime_error("Failed to read dimension from file.");
    }

    return bytes[0] | (bytes[1] << 8) | (bytes[2] << 16) | (bytes[3] << 24);
}

cv::Mat ExtractImage(std::ifstream& file, int width, int height) {
    if (width > 2000 || height > 2000) {
        std::cerr << "Image dimensions exceed the maximum allowed size of 2000x2000.\n";
        return cv::Mat(); // Return an empty matrix to indicate failure.
    }

    std::vector<unsigned char> img_data(width * height * 3);
    if (!file.read(reinterpret_cast<char*>(img_data.data()), img_data.size())) {
        std::cerr << "Failed to read image data.\n";
        return cv::Mat();
    }

    cv::Mat image(height, width, CV_8UC3, img_data.data());
    cv::Mat image_rgb;
    cv::cvtColor(image, image_rgb, cv::COLOR_BGR2RGB);
    return image_rgb.clone();
}

void ProcessFile(const std::string& file_path) {
    std::ifstream file(file_path, std::ios::binary);
    if (!file) {
        std::cerr << "Failed to open file: " << file_path << "\n";
        return;
    }

    while (FindImageHeader(file)) {
        file.ignore(1); // Skip newline character

        try {
            int width = ReadDimension(file);
            int height = ReadDimension(file);
            std::cout << "Dimension read: width=" << width << ", height=" << height << "\n";

            if (width <= 0 || height <= 0) {
                std::cerr << "Invalid image dimensions read, skipping image extraction.\n";
                continue; // Skip to the next image header if dimensions are invalid
            }

            auto image = ExtractImage(file, width, height);
            if (!image.empty()) {
                static int image_counter = 0;
                std::string output_path = fs::path(file_path).stem().string() + "_extracted_" + std::to_string(++image_counter) + ".png";
                if (cv::imwrite(output_path, image)) {
                    std::cout << "Extracted and saved image: " << output_path << "\n";
                } else {
                    std::cerr << "Failed to save the extracted image. Possible write permission issue or disk space limitation.\n";
                }
            } else {
                std::cerr << "Extracted image is empty, possible corruption or unsupported format. Skipping.\n";
            }
        } catch (const std::runtime_error& e) {
            std::cerr << "Error processing file: " << e.what() << "."
                      << " Attempting to continue with next image...\n";
        }
    }
}