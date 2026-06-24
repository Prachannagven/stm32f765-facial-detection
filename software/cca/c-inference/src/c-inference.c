#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "cca_defs.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

void print_array(uint8_t* arr, int height, int width) {
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            printf("%3d ", arr[y * width + x]);
        }
        printf("\n");
    }
}

void print_array_f(float* arr, int height, int width) {
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            printf("%3f ", arr[y * width + x]);
        }
        printf("\n");
    }
}

int main(int argc, char* argv[]) {
    /* Loading the PNG from the folder */
    int height, width, channels;
    FILE* fp = fopen("/home/pranav/PersonalProjects/kamikaze-drone/software/cca/python-training/"
                     "original-split/s01_pramit/LR/s01_1_cropped_LR.png",
                     "rb");
    uint8_t* img = stbi_load_from_file(fp, &width, &height, &channels, 1);
    if (img == NULL) {
        fprintf(stderr, "stbi_load failed: %s\n", stbi_failure_reason());
        return 1;
    }

    /* Performing Centering */
    uint8_t centered_img[width * height];

    for (int i = 0; i < width * height; i++) {
        centered_img[i] = img[i] - lr_mean[i];
    }

    /* Multiplying Against Wl */
    float projected_img[k_components];
    for (int i = 0; i < k_components; i++) {
        float sum = 0;
        for (int j = 0; j < width * height; j++) {
            sum += (float)centered_img[j] * Wl[j][i];
        }
        projected_img[i] = sum;
    }

    /* Classifying as Pramit or Not Pramit */

    /* Printing For Debugging */
    print_array(img, height, width); // Printing original image
    printf("\n\n");
    print_array(centered_img, height, width); // Printing Centered image
    printf("\n\n");
    print_array_f(projected_img, 1, k_components);

    printf("Image Size: \n\tWidth = %d \n\tHeight = %d\n", width, height);

    /* Clsoing the Pointers */
    stbi_image_free(img);
    fclose(fp);

    return 0;
}
