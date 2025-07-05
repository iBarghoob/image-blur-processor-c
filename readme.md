# Image Blur Processor

This C program applies a 3x3 blur filter to input images (PNG, JPG, ...) and saves the output as PNG files. It uses the stb_image.h and stb_image_write.h libraries for image loading and saving the images.

## Features
- Supports common image formats (PNG, JPEG, BMP, etc.) for input.
- Applies a simple 3x3 blur filter to the image.
- Outputs processed images as PNG files.
- Handles multiple image input/output pairs in a single run.

## Usage
`./blur INPUTFILE1.png OUTPUTFILE1.png [INPUTFILE2.png OUTPUTFILE2.png ...]`

Example:  
`./blur input1.jpg output1.png input2.png output2.png`

### Before
<img src="coffee.png" alt="example image" width=500/>

### After
<img src="out_coffee.png" alt="example image" width=500/>

## Process
- The program loads images using stb_image.
- Converts image data into a custom struct Image format.
- Applies a 3x3 blur by averaging the RGB values of neighboring pixels.
- Saves the processed image as a PNG file with stb_image_write.

## Output
- Output images always saved as PNG