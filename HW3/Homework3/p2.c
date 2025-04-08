#include <stdio.h>
#include <stdlib.h>

#define SIZE 3

typedef unsigned char U_CHAR;

void applyFilter(U_CHAR *input, U_CHAR *output, int width, int height, int filter[SIZE][SIZE], float c, int row_size);
void correctedPos(int *new_i, int *new_j, int i, int j, int M, int N);

int main() {
	FILE *input_file, *output_file;
	U_CHAR *input_data, *output_data1, *output_data2, *output_data3;
	U_CHAR bmpfileheader[14], bmpinfoheader[40], color_table[1024];
	int width, height, img_size, row_size, padding;

	// Ū�� BMP �ɮ�
	input_file = fopen("Fig0338(a).bmp", "rb");
	if (!input_file) {
		printf("Error: File not found!\n");
		return 1;
	}
	fread(bmpfileheader, sizeof(bmpfileheader), 1, input_file);
	fread(bmpinfoheader, sizeof(bmpinfoheader), 1, input_file);
	fread(color_table, sizeof(color_table), 1, input_file);

	width = *(int *)&bmpinfoheader[4];
	height = *(int *)&bmpinfoheader[8];
	padding = (4 - (width % 4)) % 4; // �p��C�檺��R�줸��
	row_size = width + padding;      // �C���ڤj�p�]�]�A��R�^
	img_size = row_size * height;    // �`�ƾڤj�p

	input_data = (U_CHAR *)malloc(img_size);
	output_data1 = (U_CHAR *)malloc(img_size);
	output_data2 = (U_CHAR *)malloc(img_size);
	output_data3 = (U_CHAR *)malloc(img_size);
	fread(input_data, img_size, 1, input_file);
	fclose(input_file);

	// Laplacian kernel (Fig. 3.45(a))
	int laplacian1[SIZE][SIZE] = {
		{ 0, 1,  0 },
		{1, -4, 1 },
		{ 0, 1,  0 }
	};

	// Laplacian kernel (Fig. 3.45(b))
	int laplacian2[SIZE][SIZE] = {
		{1, 1, 1 },
		{1,  -8, 1 },
		{1, 1, 1 }
	};

	// ���ΤT�ؤ�k
	applyFilter(input_data, output_data1, width, height, laplacian1, -1.0, row_size);
	applyFilter(input_data, output_data2, width, height, laplacian2, -1.0, row_size);
	applyFilter(input_data, output_data3, width, height, laplacian1, -0.5, row_size);

	// �x�s��X�v��
	output_file = fopen("p2_sharpened_laplacian1.bmp", "wb");
	fwrite(bmpfileheader, sizeof(bmpfileheader), 1, output_file);
	fwrite(bmpinfoheader, sizeof(bmpinfoheader), 1, output_file);
	fwrite(color_table, sizeof(color_table), 1, output_file);
	fwrite(output_data1, img_size, 1, output_file);
	fclose(output_file);

	output_file = fopen("p2_sharpened_laplacian2.bmp", "wb");
	fwrite(bmpfileheader, sizeof(bmpfileheader), 1, output_file);
	fwrite(bmpinfoheader, sizeof(bmpinfoheader), 1, output_file);
	fwrite(color_table, sizeof(color_table), 1, output_file);
	fwrite(output_data2, img_size, 1, output_file);
	fclose(output_file);

	output_file = fopen("p2_sharpened_unsharp.bmp", "wb");
	fwrite(bmpfileheader, sizeof(bmpfileheader), 1, output_file);
	fwrite(bmpinfoheader, sizeof(bmpinfoheader), 1, output_file);
	fwrite(color_table, sizeof(color_table), 1, output_file);
	fwrite(output_data3, img_size, 1, output_file);
	fclose(output_file);

	free(input_data);
	free(output_data1);
	free(output_data2);
	free(output_data3);

	printf("Three sharpened images generated successfully!\n");
	return 0;
}

void applyFilter(U_CHAR *input, U_CHAR *output, int width, int height, int filter[SIZE][SIZE], float c, int row_size) {
	int i, j, x, y;
	int half = SIZE / 2;
	int padding = row_size - width;

	// ��l�ƿ�X�ƾڡA�]�A��R��
	for (i = 0; i < height; i++) {
		for (j = 0; j < row_size; j++) {
			if (j >= width) output[i * row_size + j] = 0; // ��R��]�� 0
			else output[i * row_size + j] = input[i * row_size + j];
		}
	}

	// �����o�i��
	for (i = 0; i < height; i++) {
		for (j = 0; j < width; j++) { // �u�B�z���Ĺ����A���B�z��R��
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

void correctedPos(int *new_i, int *new_j, int i, int j, int M, int N) {
	*new_i = (i < 0) ? 0 : (i >= M) ? M - 1 : i;
	*new_j = (j < 0) ? 0 : (j >= N) ? N - 1 : j;
}
