#include <stdio.h>
#include <stdlib.h>

typedef long INT32;
typedef unsigned short int INT16;
typedef unsigned char U_CHAR;

#define UCH(x) ((int)(x))
#define GET_2B(array, offset) ((INT16)UCH(array[offset]) + (((INT16)UCH(array[offset + 1])) << 8))
#define GET_4B(array, offset) ((INT32)UCH(array[offset]) + (((INT32)UCH(array[offset + 1])) << 8) + (((INT32)UCH(array[offset + 2])) << 16) + (((INT32)UCH(array[offset + 3])) << 24))
#define FREAD(file, buf, sizeofbuf) ((size_t)fread((void *)(buf), (size_t)1, (size_t)(sizeofbuf), (file)))

#define NUM_POINTS 6  // 給定的6個控制點

// 目標直方圖的控制點
int control_points[NUM_POINTS][2] = {
    {0, 0}, {4, 70000}, {25, 8000}, {168, 0}, {200, 6000}, {255, 0}
};

int histo_target[256] = {0};

void compute_target_histogram() {
    int i, j;
    for (i = 0; i < NUM_POINTS - 1; i++) {
        int x1 = control_points[i][0], y1 = control_points[i][1];
        int x2 = control_points[i + 1][0], y2 = control_points[i + 1][1];

        for (j = x1; j <= x2; j++) {
            histo_target[j] = y1 + (j - x1) * (y2 - y1) / (x2 - x1);
        }
    }
}

int main() {
    FILE *input_file = NULL, *output_file = NULL;

    U_CHAR bmpfileheader[14] = {0};
    U_CHAR bmpinfoheader[40] = {0};
    U_CHAR *data, color_table[1024];
    INT32 FileSize, bfOffBits, headerSize, biWidth, biHeight;
    INT16 biPlanes, BitCount;
    INT32 biCompression, biImageSize, biXPelsPerMeter, biYPelsPerMeter, biClrUsed, biClrImp;
    int i, j, k;
    int histo_original[256] = {0};
    int histo_cdf[256] = {0}, histo_target_cdf[256] = {0};
    int mapping[256] = {0};

    // 開啟 BMP 圖片
    if ((input_file = fopen("Fig3.23(a).bmp", "rb")) == NULL) {
        fprintf(stderr, "File can't open.\n");
        return 1;
    }
    FREAD(input_file, bmpfileheader, 14);
    FREAD(input_file, bmpinfoheader, 40);
    FileSize = GET_4B(bmpfileheader, 2);
    bfOffBits = GET_4B(bmpfileheader, 10);
    headerSize = GET_4B(bmpinfoheader, 0);
    biWidth = GET_4B(bmpinfoheader, 4);
    biHeight = GET_4B(bmpinfoheader, 8);
    biPlanes = GET_2B(bmpinfoheader, 12);
    BitCount = GET_2B(bmpinfoheader, 14);
    biCompression = GET_4B(bmpinfoheader, 16);
    biImageSize = GET_4B(bmpinfoheader, 20);
    biXPelsPerMeter = GET_4B(bmpinfoheader, 24);
    biYPelsPerMeter = GET_4B(bmpinfoheader, 28);
    biClrUsed = GET_4B(bmpinfoheader, 32);
    biClrImp = GET_4B(bmpinfoheader, 36);

    if (BitCount != 8) {
        fprintf(stderr, "Not an 8-bit file.\n");
        fclose(input_file);
        return 1;
    }

    FREAD(input_file, color_table, 1024);
    data = (U_CHAR *)malloc(((biWidth + 3) / 4 * 4) * biHeight);
    if (data == NULL) {
        fprintf(stderr, "Insufficient memory.\n");
        fclose(input_file);
        return 1;
    }

    fseek(input_file, bfOffBits, SEEK_SET);
    FREAD(input_file, data, ((biWidth + 3) / 4 * 4) * biHeight);
    fclose(input_file);
    for (i = 0; i < biHeight; i++) {
        k = i * ((biWidth + 3) / 4 * 4);
        for (j = 0; j < biWidth; j++) {
            histo_original[data[k]]++;
            k++;
        }
    }

    // 計算原始影像的 CDF
    histo_cdf[0] = histo_original[0];
    for (i = 1; i < 256; i++) {
        histo_cdf[i] = histo_cdf[i - 1] + histo_original[i];
    }

    // 計算目標直方圖
    compute_target_histogram();

    // 計算目標 CDF
    histo_target_cdf[0] = histo_target[0];
    for (i = 1; i < 256; i++) {
        histo_target_cdf[i] = histo_target_cdf[i - 1] + histo_target[i];
    }

    // 標準化 CDF
    for (i = 0; i < 256; i++) {
        histo_cdf[i] = (histo_cdf[i] * 255) / histo_cdf[255];
        histo_target_cdf[i] = (histo_target_cdf[i] * 255) / histo_target_cdf[255];
    }

    // 建立灰階值對應映射
    for (i = 0; i < 256; i++) {
        int min_diff = 1e9, best_match = 0;
        for (j = 0; j < 256; j++) {
            int diff = abs(histo_cdf[i] - histo_target_cdf[j]);
            if (diff < min_diff) {
                min_diff = diff;
                best_match = j;
            }
        }
        mapping[i] = best_match;
    }

    // 應用直方圖匹配
    for (i = 0; i < biHeight; i++) {
        k = i * ((biWidth + 3) / 4 * 4);
        for (j = 0; j < biWidth; j++) {
            data[k] = mapping[data[k]];
            k++;
        }
    }

    // 輸出新的 BMP 圖片
    if ((output_file = fopen("p1_output.bmp", "wb")) == NULL) {
        fprintf(stderr, "Output file can't open.\n");
        return 1;
    }

    fwrite(bmpfileheader, sizeof(bmpfileheader), 1, output_file);
    fwrite(bmpinfoheader, sizeof(bmpinfoheader), 1, output_file);
    fwrite(color_table, 1024, 1, output_file);
    fwrite(data, ((biWidth + 3) / 4 * 4) * biHeight, 1, output_file);
    fclose(output_file);

    free(data);
    return 0;
}

