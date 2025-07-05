#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

/* The RGB values of a pixel. */
struct Pixel {
    unsigned char red;
    unsigned char green;
    unsigned char blue;
};

/* An image loaded from a file. */
struct Image {
    int width;
    int height;
    struct Pixel *pixels;
    const char *output_filename;
};

/* Free a struct Image */
void free_image(struct Image *img)
{
    if (img != NULL){
        if (img -> pixels != NULL){
            free(img -> pixels);
        }
        free(img);
    }
}

/* Opens and reads an image file, returning a pointer to a new struct Image. */
struct Image *load_image(const char *filename, const char *output_filename)
{
    int width, height, channels;
    unsigned char *data = stbi_load(filename, &width, &height, &channels, 3);
    if (!data) {
        fprintf(stderr, "Failed to load image: %s\n", filename);
        return NULL;
    }

    struct Image *img = malloc(sizeof(struct Image));
    img->width = width;
    img->height = height;
    img->output_filename = output_filename;
    img->pixels = malloc(width * height * sizeof(struct Pixel));

    for (int i = 0; i < width * height; i++) {
        img->pixels[i].red   = data[i * 3 + 0];
        img->pixels[i].green = data[i * 3 + 1];
        img->pixels[i].blue  = data[i * 3 + 2];
    }

    stbi_image_free(data);
    return img;
}

/* Write img to file filename. */
bool save_image(const struct Image *img)
{
    int width = img->width;
    int height = img->height;

    unsigned char *data = malloc(width * height * 3);
    if (!data) return false;

    for (int i = 0; i < width * height; i++) {
        data[i * 3 + 0] = img->pixels[i].red;
        data[i * 3 + 1] = img->pixels[i].green;
        data[i * 3 + 2] = img->pixels[i].blue;
    }

    // save as PNG regardless of extension
    int result = stbi_write_png(img->output_filename, width, height, 3, data, width * 3);
    free(data);

    if (result == 0) {
        fprintf(stderr, "Failed to write PNG file: %s\n", img->output_filename);
        return false;
    }

    return true;
}

/* Allocates a new Image struct and copy an existing Image's contents
 * into it. On error, returns NULL. */
struct Image *copy_image(const struct Image *source)
{
    if (source == NULL){
        fprintf(stderr, "Error in processing source file.\n");
        return NULL;
    }

    // allocate memory for new struct
    struct Image *image_copy = malloc(sizeof(struct Image));
    if (image_copy == NULL){
        fprintf(stderr, "Failed to allocate memory for copying image.\n");
        return NULL;
    }

    int width = source -> width;
    int height = source -> height;

    // copy source image dimensions to image copy
    image_copy -> width = width;
    image_copy -> height = height;

    // allocate memory for pixel data
    image_copy -> pixels = malloc(width * height * sizeof(struct Pixel));
    if (image_copy -> pixels == NULL){
        fprintf(stderr, "Failed to allocate memory for copied image pixels.\n");
        free(image_copy);
        return NULL;
    }

    // loop through pixels in image and set the copied image pixels to the source image pixels
    // the pixel data is stored as a struct so the rgb values are copied to the corresponding pixel in copied image
    for (int i = 0; i < width * height; i++){
        image_copy -> pixels[i] = source -> pixels[i]; 
    }

    // copy output filename from source image
    image_copy -> output_filename = source -> output_filename;
    return image_copy;
}

/* Applies a 3x3 blue to an image.
 * It takes as input a pointer to the source image struct.
 * Returns a poin struct Image containing the result, or NULL on error. */
struct Image *apply_BLUR(const struct Image *source)
{
    if (source == NULL){
        fprintf(stderr, "Invalid source image.\n");
        return NULL;
    }

    // create a copy of the source image
    struct Image *blurred_image = copy_image(source);
    if (blurred_image == NULL){
        fprintf(stderr, "Failed to copy source image.\n");
        return NULL;
    }

    // image dimensions
    int width = source -> width;
    int height = source -> height;

    // loop over each pixel in the image
    for (int row = 0; row < height; row++){
        for (int col = 0; col < width; col++){
            // sum of RGB values in image
            int red_sum = 0, green_sum = 0, blue_sum = 0;
            int valid_pixels = 0;

            // loop the 3x3 block surrounding the current pixel
            for (int i = -1; i <= 1; i++){
                for (int j = -1; j <= 1; j++){
                    // get the row and column indices of the adjacent pixel
                    int adjacent_row = row + i;
                    int adjacent_col = col + j;

                    // check if adjacent pixel is within the bounds of the image
                    if (adjacent_row >= 0 && adjacent_row < height && adjacent_col >= 0 && adjacent_col < width) {
                        // get the current pixel
                        struct Pixel *pixel = &source -> pixels[(row + i) * width + (j + col)];

                        // sum the RGB values of the 3x3 block around the pixel
                        red_sum += pixel -> red;
                        blue_sum += pixel -> blue;
                        green_sum += pixel -> green;
                        valid_pixels++;
                    }
                }
            }
            // get pixel in blurred image and set the RGB values to the average of the sums
            struct Pixel *blurred_pixel = &blurred_image -> pixels[row * width + col];
            blurred_pixel -> red = red_sum / valid_pixels;
            blurred_pixel -> blue = blue_sum / valid_pixels;
            blurred_pixel -> green = green_sum / valid_pixels;
        }
    }
    return blurred_image;
}

int main(int argc, char *argv[])
{
    /* Check command-line arguments */
    if (argc < 3) {
        fprintf(stderr, "Usage: process INPUTFILE1 OUTPUTFILE1 INPUTFILE2 OUTPUTFILE2 ...\n");
        return 1;
    }

    // get number of input output pairs (images to process)
    int num_images = (argc - 1) / 2; 

    // allocate memory for an array of pointers to struct Image 
    struct Image **images = malloc(num_images * sizeof(struct Image *));
    if (images == NULL) {
        fprintf(stderr, "Memory allocation failed.\n");
        return 1;
    }

    // load input images
    for (int i = 0; i < num_images; i++){
        // get input and output filename for each image pair
        const char *input_filename = argv[i * 2 + 1];
        const char *output_filename = argv[i * 2 + 2];

        // load the image from input file and allocate memory for the Image struct
        struct Image *img = load_image(input_filename, output_filename);
        if (img == NULL) {
            // free memory allocated for previous images if loading fails
            for (int j = 0; j < i; j++) {
                free_image(images[j]);
            }
            free(images);
            return 1;
        }
        // store the pointer to the loaded image in the array images
        images[i] = img;
    }
    
    // apply blur
    for (int i = 0; i < num_images; i++) {
        struct Image *out_img = apply_BLUR(images[i]);

        // free memory allocated for previous images if processing fails
        if (out_img == NULL) {
            fprintf(stderr, "BLUR failed for image %d.\n", i + 1);
            for (int j = 0; j <= i; j++) {
                free_image(images[j]);
            }
            free(images);
            return 1;
        }

        // free memory of the pointer to original image and replace it with the pointer to the processed image
        free_image(images[i]);
        images[i] = out_img;
    }

    // save output images 
    for (int i = 0; i < num_images; i++) {
        // save processed image to the output file
        if (!save_image(images[i])) {
            fprintf(stderr, "Saving image %d failed.\n", i + 1);
            // free memory allocated for previous images if saving fails
            for (int j = 0; j <= i; j++) {
                free_image(images[j]);
            }
            free(images);
            return 1;
        }
        free_image(images[i]);
    }

    free(images);
    return 0;
}