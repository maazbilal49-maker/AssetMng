#include <iostream>
#include <vector>
#include <string.h>
#include <stdlib.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include <stb/stb_image_resize2.h>

using namespace std;

//green to detect (49, 255, 109)
void remove_color(const char* filename){
    int width, height, channels;

    unsigned char* data = stbi_load(filename, &width, &height, &channels, 4);

    if(data == nullptr){
        cout << "Failed to load image." << '\n';
        return;
    }

    for(int y = 0; y < height; y++){
        for(int x = 0; x < width; x++){
            int pixelIndex = (y * width + x) * 4;

            unsigned char r = data[pixelIndex];
            unsigned char g = data[pixelIndex + 1];
            unsigned char b = data[pixelIndex + 2];
            unsigned char a = data[pixelIndex + 3];

            if(r == 49 && g == 255 && b == 109){
                data[pixelIndex + 3] = 0;
            }
        }
    }

    stbi_write_png(filename, width, height, 4, data, width * 4);

    stbi_image_free(data);
}

void resize_image(const char *filename, int new_w, int new_h){
    int width, height, channels;
    unsigned char *data = stbi_load(filename, &width, &height, &channels, 4);

    if(data == nullptr){
        cout << "Could not open file." << '\n';
        return;
    }

    unsigned char *new_data = (unsigned char*)malloc(new_w * new_h * 4);

    if(!new_data){
        cout << "Could not allocate memory." << '\n';
        stbi_image_free(data);
        return;
    }

    stbir_resize_uint8_srgb(data, width, height, 0,
                            new_data, new_w, new_h, 0,
                            STBIR_4CHANNEL);

    stbi_write_png(filename, new_w, new_h, channels, new_data, new_w * 4);

    stbi_image_free(data);
    free(new_data);
}

void flip_image_vertical(const char *filename){
    stbi_set_flip_vertically_on_load(true);

    int width, height, channels;
    unsigned char *data = stbi_load(filename, &width, &height, &channels, 4);
    
    if(data == nullptr){
        cout << "Failed to open file." << '\n';
        return;
    }

    stbi_write_png(
        filename,
        width,
        height,
        4,
        data,
        width * 4
    );

    stbi_image_free(data);

    stbi_set_flip_vertically_on_load(false);
}

void flip_image_horizontal(const char *filename){
    int width, height, channels;
    unsigned char *data = stbi_load(filename, &width, &height, &channels, 4);

    if(data == nullptr){
        cout << "Failed to open file." << '\n';
        return;
    }

    int row, col;
    int row_size = width * channels;

    for (row = 0; row < height; row++) {
        unsigned char *current_row = data + (row * row_size);
        
        // Swap pixels from left to right
        for (col = 0; col < width / 2; col++) {
            int left_idx = col * channels;
            int right_idx = (width - 1 - col) * channels;

            // Swap each channel for the pixel pair
            for (int c = 0; c < channels; c++) {
                unsigned char temp = current_row[left_idx + c];
                current_row[left_idx + c] = current_row[right_idx + c];
                current_row[right_idx + c] = temp;
            }
        }
    }

    stbi_write_png(filename, width, height, channels, data, width * channels);

    stbi_image_free(data);
}

void crop_image(const char *filename, int x, int y, int crop_width, int crop_height){
    int width, height, channels;
    unsigned char *data = stbi_load(filename, &width, &height, &channels, 4);

    if(data == nullptr){
        cout << "Failed to open file." << '\n';
        return;
    }

    if(x < 0 || y < 0 || crop_width <= 0 || crop_height <= 0 ||
       x + crop_width > width || y + crop_height > height){
        cout << "Crop rectangle is outside the image." << '\n';
        stbi_image_free(data);
        return;
    }

    vector<unsigned char> cropped_data(crop_width * crop_height * 4);

    for(int row = 0; row < crop_height; row++){
        const unsigned char *source = data + ((y + row) * width + x) * 4;
        unsigned char *destination = cropped_data.data() + row * crop_width * 4;

        memcpy(destination, source, crop_width * 4);
    }

    stbi_write_png(filename, crop_width, crop_height, 4,
                   cropped_data.data(), crop_width * 4);

    stbi_image_free(data);
}

int main(int argc, char *argv[]){
    if(argc < 2){
        cout << "Use: ./assetmng <filename>" << endl;
        return 1;
    }

    if(strcmp(argv[1], "--remove-color") == 0)
        for(int i = 2; i < argc; i++){
            remove_color(argv[i]);
        }
    
    else if(strcmp(argv[1], "--resize") == 0){
        if(argc < 5){
            cout << "Use: ./assetmng --resize <filename> <width> <height>" << endl;
            return 1;
        }

        char *filename = argv[2];
        int width = atoi(argv[3]);
        int height = atoi(argv[4]);

        resize_image(filename, width, height);
    }

    else if(strcmp(argv[1], "--crop") == 0){
        if(argc < 7){
            cout << "Use: ./assetmng --crop <filename> <x> <y> <width> <height>" << endl;
            return 1;
        }

        crop_image(argv[2], atoi(argv[3]), atoi(argv[4]),
                   atoi(argv[5]), atoi(argv[6]));
    }

    else if(strcmp(argv[1], "--flip") == 0){
        if(argc < 4){
            cout << "Use: ./assetmng --flip <filename> <horizontal|vertical>" << endl;
            return 1;
        }

        char *filename = argv[2];
        char *mode = argv[3];
        if(strcmp(mode, "horizontal") == 0){
            flip_image_horizontal(filename);
        }
        else if(strcmp(mode, "vertical") == 0){
            flip_image_vertical(filename);
        }
        else{
            cout << "Invalid flip mode." << '\n';
            return 1;
        }
    }

    else{
        cout << "Invalid command." << '\n';
        return 1;
    }

    return 0;
}