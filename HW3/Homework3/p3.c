#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define SIZE 3
#define GAMMA 0.5  // Power-law transformation gamma 值

typedef unsigned char U_CHAR;

void applyLaplacian(U_CHAR *input, U_CHAR *output, int width, int height, int filter[SIZE][SIZE], float c, int row_size);
void applySobel(U_CHAR *input, U_CHAR *output, int width, int height, int row_size);
void applyAveraging(U_CHAR *input, U_CHAR *output, int width, int height, int row_size);
void applyPowerLaw(U_CHAR *input, U_CHAR *output, int width, int height, int row_size, float gamma);
void correctedPos(int *new_i, int *new_j, int i, int j, int M, int N);

int main() {
    FILE *input_file, *output_file;
    U_CHAR *input_data, *laplacian_data, *sum1_data, *sobel_data, *smoothed_sobel, *product_data, *sum2_data, *final_data;
    U_CHAR bmpfileheader[14], bmpinfoheader[40], color_table[1024];
    int width, height, img_size, row_size, padding;

    // 讀取 BMP 檔案
    input_file = fopen("Fig3.43(a).bmp", "rb");
    if (!input_file) {
        printf("Error: File not found!\n");
        return 1;
    }
    fread(bmpfileheader, sizeof(bmpfileheader), 1, input_file);
    fread(bmpinfoheader, sizeof(bmpinfoheader), 1, input_file);
    fread(color_table, sizeof(color_table), 1, input_file);

    width = *(int *)&bmpinfoheader[4];
    height = *(int *)&bmpinfoheader[8];
    padding = (4 - (width % 4)) % 4;
    row_size = width + padding;
    img_size = row_size * height;

    input_data = (U_CHAR *)malloc(img_size);
    laplacian_data = (U_CHAR *)malloc(img_size);
    sum1_data = (U_CHAR *)malloc(img_size);
    sobel_data = (U_CHAR *)malloc(img_size);
    smoothed_sobel = (U_CHAR *)malloc(img_size);
    product_data = (U_CHAR *)malloc(img_size);
    sum2_data = (U_CHAR *)malloc(img_size);
    final_data = (U_CHAR *)malloc(img_size);

    fread(input_data, img_size, 1, input_file);
    fclose(input_file);

    // Laplacian kernel
    int laplacian_filter[SIZE][SIZE] = {
        {0, 1, 0},
        {1, -4, 1},
        {0, 1, 0}
    };
    applyLaplacian(input_data, laplacian_data, width, height, laplacian_filter, -1.0, row_size);
    int i;
    // Sum1 = A + Laplacian(A)
    for (i = 0; i < img_size; i++) {
        sum1_data[i] = (U_CHAR)(input_data[i] + laplacian_data[i]);
    }
    
    // Sobel gradient
    applySobel(input_data, sobel_data, width, height, row_size);
    
    // Smoothed Sobel
    applyAveraging(sobel_data, smoothed_sobel, width, height, row_size);
    
    // Product = Sum1 * Smoothed Sobel
    for (i = 0; i < img_size; i++) {
        int value = (sum1_data[i] * smoothed_sobel[i]) / 255;
        product_data[i] = (U_CHAR)((value > 255) ? 255 : value);
    }
    
    // Sum2 = A + Product
    for (i = 0; i < img_size; i++) {
        sum2_data[i] = (U_CHAR)(input_data[i] + product_data[i]);
    }
    
    // Applying Power-law transformation
    applyPowerLaw(sum2_data, final_data, width, height, row_size, GAMMA);
    
    // 儲存最終影像
    output_file = fopen("final_output.bmp", "wb");
    fwrite(bmpfileheader, sizeof(bmpfileheader), 1, output_file);
    fwrite(bmpinfoheader, sizeof(bmpinfoheader), 1, output_file);
    fwrite(color_table, sizeof(color_table), 1, output_file);
    fwrite(final_data, img_size, 1, output_file);
    fclose(output_file);

    free(input_data);
    free(laplacian_data);
    free(sum1_data);
    free(sobel_data);
    free(smoothed_sobel);
    free(product_data);
    free(sum2_data);
    free(final_data);

    printf("Final power-law transformed image generated successfully!\n");
    return 0;
}

// Laplacian 運算
void applyLaplacian(U_CHAR *input, U_CHAR *output, int width, int height, int filter[3][3], float c, int row_size) {
    int i, j, x, y;
    int half = 3 / 2; 
    int padding = row_size - width;
    for (i = 0; i < height; i++) {
        for (j = 0; j < row_size; j++) {
            if (j >= width) output[i * row_size + j] = 0;
            else output[i * row_size + j] = input[i * row_size + j];
        }
    }
    for (i = 0; i < height; i++) {
        for (j = 0; j < width; j++) {
            float sum = 0;
            for (x = -half; x <= half; x++) {
                for (y = -half; y <= half; y++) {
                    int ni, nj;
                    correctedPos(&ni, &nj, i + x, j + y, height, width);
                    sum += input[ni * row_size + nj] * filter[x + half][y + half];
                }
            }
            float value = input[i * row_size + j] + c * sum;
            if (value < 0) value = 0;
            if (value > 255) value = 255;
            output[i * row_size + j] = (U_CHAR)value;
        }
    }
}

// Sobel 運算
void applySobel(U_CHAR *input, U_CHAR *output, int width, int height, int row_size) {
    int sobelX[3][3] = {{-1, 0, 1}, {-2, 0, 2}, {-1, 0, 1}};
    int sobelY[3][3] = {{-1, -2, -1}, {0, 0, 0}, {1, 2, 1}};
    int i,j,x,y;
    for (i = 1; i < height - 1; i++) {
        for (j = 1; j < width - 1; j++) {
            int Gx = 0, Gy = 0;
            for (x = -1; x <= 1; x++) {
                for (y = -1; y <= 1; y++) {
                    Gx += input[(i + x) * row_size + (j + y)] * sobelX[x + 1][y + 1];
                    Gy += input[(i + x) * row_size + (j + y)] * sobelY[x + 1][y + 1];
                }
            }
            output[i * row_size + j] = (U_CHAR)(sqrt(Gx * Gx + Gy * Gy) > 255 ? 255 : sqrt(Gx * Gx + Gy * Gy));
        }
    }
}

// 5×5 均值濾波
void applyAveraging(U_CHAR *input, U_CHAR *output, int width, int height, int row_size) {
    int i,j,x,y;
	for (i = 2; i < height - 2; i++) {
        for (j = 2; j < width - 2; j++) {
            int sum = 0;
            for (x = -2; x <= 2; x++) {
                for (y = -2; y <= 2; y++) {
                    sum += input[(i + x) * row_size + (j + y)];
                }
            }
            output[i * row_size + j] = sum / 25;
        }
    }
}

// Power-Law 變換
void applyPowerLaw(U_CHAR *input, U_CHAR *output, int width, int height, int row_size, float gamma) {
    int i;
	for (i = 0; i < width * height; i++) {
        float pixel = (float)input[i] / 255.0;
        pixel = powf(pixel, gamma);
        output[i] = (U_CHAR)(pixel * 255.0);
    }
}

// 修正邊界處理
void correctedPos(int *new_i, int *new_j, int i, int j, int M, int N) {
    *new_i = (i < 0) ? 0 : (i >= M) ? M - 1 : i;
    *new_j = (j < 0) ? 0 : (j >= N) ? N - 1 : j;
}

