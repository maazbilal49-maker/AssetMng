# AssetMng

AssetMng is a command-line image processing tool written in **C++**. It provides basic image manipulation functionality through a simple command-line interface.

## Technologies

### C++

The project is written in C++ and uses the standard library for general-purpose functionality.

Used standard library features include:

* `iostream` — console input/output
* `vector` — dynamic memory management for image data
* `string.h` — string comparison and memory operations
* `stdlib.h` — memory allocation and string-to-integer conversion

### stb_image

**stb_image** is used to load image files into raw pixel data.

Images are loaded as four-channel RGBA data:

```cpp
unsigned char* data = stbi_load(
    filename,
    &width,
    &height,
    &channels,
    4
);
```

This allows the image-processing functions to work with a consistent pixel format:

```text
Red → Green → Blue → Alpha
```

After processing, the image memory is released using `stbi_image_free()`.

### stb_image_write

**stb_image_write** is used to save the processed image data back to PNG files.

```cpp
stbi_write_png(
    filename,
    width,
    height,
    4,
    data,
    width * 4
);
```

The program modifies the image data in memory and then writes the resulting pixels back to the file.

### stb_image_resize2

**stb_image_resize2** is used for resizing images.

The original pixel buffer is loaded and a second buffer is allocated for the resized image. `stbir_resize_uint8_srgb()` then performs the resizing operation.

```cpp
stbir_resize_uint8_srgb(
    data,
    width,
    height,
    0,
    new_data,
    new_w,
    new_h,
    0,
    STBIR_4CHANNEL
);
```

The newly resized buffer is then written back to the image file.

---

# How It Works

## Image Representation

Images are represented as a contiguous array of `unsigned char` values.

Because AssetMng loads images as RGBA, every pixel occupies **4 bytes**:

```text
[R][G][B][A]
```

The location of a pixel can be calculated using:

```cpp
int pixelIndex = (y * width + x) * 4;
```

This converts the two-dimensional `(x, y)` coordinate into an index in the one-dimensional pixel array.

For example, the four values beginning at `pixelIndex` represent:

```text
pixelIndex + 0 → Red
pixelIndex + 1 → Green
pixelIndex + 2 → Blue
pixelIndex + 3 → Alpha
```

---

## Color Removal

The color removal system iterates through every pixel in the image.

It compares the RGB channels against a predefined color:

```text
(49, 255, 109)
```

When a matching pixel is found, its alpha channel is set to zero:

```cpp
data[pixelIndex + 3] = 0;
```

An alpha value of `0` makes the pixel completely transparent.

The RGB values are left unchanged.

---

## Image Resizing

The resizing process uses two image buffers.

First, the original image is loaded into memory.

A second buffer is allocated based on the requested dimensions:

```cpp
new_w * new_h * 4
```

The resize library then calculates the new pixel data and stores it in this second buffer.

The original buffer is eventually freed, and the resized buffer is written to the image file.

---

## Horizontal Flipping

Horizontal flipping is performed directly on the loaded pixel buffer.

Each row is processed independently.

Pixels are swapped from opposite sides of the row:

```text
Before:

A B C D E

After:

E D C B A
```

The program swaps every channel of the two pixels individually.

The process continues until the middle of the row is reached.

This means the operation does not require a second image buffer.

---

## Vertical Flipping

Vertical flipping uses functionality provided by `stb_image`.

The program enables vertical flipping when loading the image:

```cpp
stbi_set_flip_vertically_on_load(true);
```

The image is therefore loaded in vertically reversed order.

It is then written back to the file and the setting is disabled afterward:

```cpp
stbi_set_flip_vertically_on_load(false);
```

---

## Cropping

Cropping creates a new pixel buffer containing only the selected region of the original image.

Before copying anything, the program verifies that the requested rectangle is inside the image boundaries.

For every row of the crop, a section of the original pixel buffer is copied into the new buffer using `memcpy()`.

Conceptually:

```text
Original image:

+-----------------------+
|                       |
|    +-------------+    |
|    |    Crop     |    |
|    |   region    |    |
|    +-------------+    |
|                       |
+-----------------------+

              ↓

+-------------+
|    Crop     |
|   region    |
+-------------+
```

The resulting buffer is then written as the new image.

The crop buffer uses `std::vector<unsigned char>`, allowing C++ to automatically manage its memory.

---

## Memory Management

AssetMng works directly with dynamically allocated image buffers.

Memory returned by `stbi_load()` is released with:

```cpp
stbi_image_free(data);
```

The resize operation manually allocates its destination buffer with:

```cpp
malloc()
```

and releases it with:

```cpp
free()
```

The crop operation instead uses:

```cpp
std::vector<unsigned char>
```

which automatically releases its memory when the vector goes out of scope.

This combination allows the project to demonstrate both **manual memory management** and **RAII-based C++ memory management**.

---

## Command-Line Interface

The program receives its operations through command-line arguments.

The first argument determines the requested operation:

```cpp
argv[1]
```

Additional arguments contain information such as:

* Input filename
* Image dimensions
* Crop coordinates
* Crop dimensions
* Flip direction

The program uses `strcmp()` to determine which operation was requested and then calls the corresponding image-processing function.

This keeps the image-processing logic separated into individual functions such as:

```cpp
remove_color()
resize_image()
flip_image_horizontal()
flip_image_vertical()
crop_image()
```

Each function is responsible for one specific image operation.
