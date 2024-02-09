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
#include <array>
#include <string_view>

namespace fs = std::filesystem;

struct ImageConfig {

	static constexpr std::array<std::string_view, 1> Headers = {"Image8"};
	
    static constexpr int MaxWidth = 2000;
    static constexpr int MaxHeight = 2000;

};

class ImageFile {

public:
    static bool FindHeader(
        std::ifstream& file
    );

    static std::pair<int, int> ReadDimensions(
        std::ifstream& file
    );

    static void SaveAsBMP(
        const fs::path& output_path,
        const std::vector<unsigned char>& img_data,
        int width,
        int height
    );

    static void Process(
        const fs::path& file_path
    );
};

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <file_path>\n";
        return 1;
    }

    ImageFile::Process(argv[1]);

    return 0;
}

bool ImageFile::FindHeader(std::ifstream& file) {
    std::string buffer;
    char ch;

    while (file.get(ch)) {
        buffer.push_back(ch);
        for (auto header : ImageConfig::Headers) {
            if (buffer.size() > header.size()) buffer.erase(buffer.begin());
            if (buffer == header) return true;
        }
        if (buffer.size() > ImageConfig::Headers[0].length()) buffer.erase(buffer.begin());
    }
    return false;
}

std::pair<int, int> ImageFile::ReadDimensions(std::ifstream& file) {
    unsigned char bytes[4];
    int width, height;

    for (int i = 0; i < 2; ++i) {
        if (!file.read(reinterpret_cast<char*>(bytes), sizeof(bytes))) {
            throw std::runtime_error("Failed to read dimensions from file.");
        }
        int& dimension = (i == 0) ? width : height;
        dimension = bytes[0] | (bytes[1] << 8) | (bytes[2] << 16) | (bytes[3] << 24);
    }

    return {width, height};
}

void ImageFile::SaveAsBMP(
    const fs::path& output_path,
    const std::vector<unsigned char>& img_data,
    int width,
    int height
) 

{
    std::ofstream file(output_path, std::ios::binary);
    if (!file) {
        std::cerr << "Failed to open output file.\n";
        return;
}

// Set up BMP file header and info header with appropriate values for a BMP image.
// Adjust padding for each row based on width to ensure proper alignment.
// Write the headers to the file stream.

    unsigned char bmpFileHeader[14] = {'B', 'M', 0, 0, 0, 0, 0, 0, 0, 0, 54, 0, 0, 0};
    unsigned char bmpInfoHeader[40] = {40, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 24, 0};
    int paddingAmount = (4 - (width * 3) % 4) % 4;
    int fileSize = 54 + (width * 3 + paddingAmount) * height;

    bmpFileHeader[2] = fileSize;
    bmpFileHeader[3] = fileSize >> 8;
    bmpFileHeader[4] = fileSize >> 16;
    bmpFileHeader[5] = fileSize >> 24;

    bmpInfoHeader[4] = width;
    bmpInfoHeader[5] = width >> 8;
    bmpInfoHeader[6] = width >> 16;
    bmpInfoHeader[7] = width >> 24;
    bmpInfoHeader[8] = height;
    bmpInfoHeader[9] = height >> 8;
    bmpInfoHeader[10] = height >> 16;
    bmpInfoHeader[11] = height >> 24;

    file.write(reinterpret_cast<char*>(bmpFileHeader), sizeof(bmpFileHeader));
    file.write(reinterpret_cast<char*>(bmpInfoHeader), sizeof(bmpInfoHeader));

// Create padding array to ensure proper alignment of pixel data.
// Iterate over each row of the image data, swapping red and blue channels for each pixel.
// Write the modified image row to the file stream, followed by padding to align to 4-byte boundaries.

    unsigned char padding[3] = {0, 0, 0};
    for (int i = height - 1; i >= 0; --i) {
        for (int j = 0; j < width; ++j) {
			unsigned char& pixel_red = const_cast<unsigned char&>(img_data[(i * width * 3) + (j * 3)]);
			unsigned char& pixel_blue = const_cast<unsigned char&>(img_data[(i * width * 3) + (j * 3) + 2]);
			std::swap(pixel_red, pixel_blue);
        }
        file.write(reinterpret_cast<const char*>(img_data.data() + (i * width * 3)), width * 3);
        file.write(reinterpret_cast<const char*>(padding), paddingAmount);
    }
}

// Try to read dimensions of the image; if unsuccessful, continue to the next header.
// Check if image dimensions exceed maximum allowed values; if so, skip processing.
// Allocate memory for image data and read the pixel data from the file.

void ImageFile::Process(const fs::path& file_path) {
    std::ifstream file(file_path, std::ios::binary);
    if (!file) return;

    static int image_counter = 0;
    while (FindHeader(file)) {
        file.ignore(1);
        try {
            auto [width, height] = ReadDimensions(file);
            if (width > ImageConfig::MaxWidth || height > ImageConfig::MaxHeight) continue;

            std::vector<unsigned char> img_data(width * height * 3);
            if (!file.read(reinterpret_cast<char*>(img_data.data()), img_data.size())) continue;

            fs::path output_path = file_path.stem().string() + "_extracted_" + std::to_string(++image_counter) + ".bmp";
            SaveAsBMP(output_path, img_data, width, height);
        } catch (const std::runtime_error&) {
            continue;
        }
    }
}
