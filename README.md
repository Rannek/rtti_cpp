# RTTI Image Extractor in C++
Raw Therapee Thumbnail Extractor in C++ supporting binary files

## Dependencies

- OpenCV (opencv2/opencv.hpp)
- C++ standard libraries: `<cstdint>`, `<filesystem>`, `<fstream>`, `<iostream>`, `<vector>`, `<stdexcept>`

## Functionality

- **FindImageHeader**: Searches for the "Image8" header within the binary file.
- **ReadDimension**: Reads the width and height dimensions of the image.
- **ExtractImage**: Extracts the image data and converts it to RGB format.
- **ProcessFile**: Main function to process the provided file path, extract images, and save them.


1. **Compile Source Code:**  
   Compile the provided source code using make command or use the precompiled program.

2. **Testing:**  
You can test the utility using the provided "TEST" folder.

Usage: `./thumbnail_extractor img.bin`

**Note:** Currently only working with `Image8` headers

*Any contributions are welcome*
