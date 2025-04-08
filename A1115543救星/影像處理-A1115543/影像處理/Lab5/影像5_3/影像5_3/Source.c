#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#pragma warning(disable:4996)

typedef long INT32;
typedef unsigned short int INT16;
typedef unsigned char U_CHAR;

#define UCH(x)	((int) (x))
#define GET_2B(array,offset)  ((INT16) UCH(array[offset]) + \
			       (((INT16) UCH(array[offset+1])) << 8))
#define GET_4B(array,offset)  ((INT32) UCH(array[offset]) + \
			       (((INT32) UCH(array[offset+1])) << 8) + \
			       (((INT32) UCH(array[offset+2])) << 16) + \
			       (((INT32) UCH(array[offset+3])) << 24))
#define FREAD(file,buf,sizeofbuf)  \
  ((size_t) fread((void *) (buf), (size_t) 1, (size_t) (sizeofbuf), (file)))

int ReadDataSize(char* name);
void ReadImageData(char* name, U_CHAR* bmpfileheader, U_CHAR* bmpinfoheader, U_CHAR* color_table, U_CHAR* data);

#define SIZE 3
#define Q1  1.5
#define Q2 -1.5
void correctedPos(int* new_i, int* new_j, int i, int j, int M, int N);

void apply_contraharmonic_filter(U_CHAR* input, U_CHAR* output, int width, int height, double Q);

int main()
{
	FILE* output_file = 0;
	U_CHAR bmpfileheader[14] = { 0 };
	U_CHAR bmpinfoheader[40] = { 0 };
	U_CHAR* data1, * data2, color_table[1024];

	//讀入第一張照片(1.bmp)=>pepper
	int dataSize = ReadDataSize("1.bmp");
	data1 = (U_CHAR*)malloc(dataSize);
	if (data1 == NULL) {
		exit(0);
	}
	ReadImageData("1.bmp", bmpfileheader, bmpinfoheader, color_table, data1);

	int biWidth = GET_4B(bmpinfoheader, 4);
	int biHeight = GET_4B(bmpinfoheader, 8);

	data2 = (U_CHAR*)malloc(dataSize);
	if (data2 == NULL) {
		exit(0);
	}
	//產生第一個消除pepper noise的圖片(result_pepper)，Q帶入Q1=1.5
	apply_contraharmonic_filter(data1, data2, biWidth, biHeight, Q1);

	if ((output_file = fopen("result_pepper.bmp", "wb")) == NULL) {
		fprintf(stderr, "Output file can't open.\n");
		exit(0);
	}
	fwrite(bmpfileheader, sizeof(bmpfileheader), 1, output_file);
	fwrite(bmpinfoheader, sizeof(bmpinfoheader), 1, output_file);
	fwrite(color_table, 1024, 1, output_file);
	fwrite(data2, ((biWidth * 1 + 3) / 4 * 4) * biHeight * 1, 1, output_file);
	fclose(output_file);

	free(data1);
	free(data2);

	//讀入第二張照片(2.bmp)=>salt
	dataSize = ReadDataSize("2.bmp");
	data1 = (U_CHAR*)malloc(dataSize);
	if (data1 == NULL) {
		exit(0);
	}
	ReadImageData("2.bmp", bmpfileheader, bmpinfoheader, color_table, data1);

	data2 = (U_CHAR*)malloc(dataSize);
	if (data2 == NULL) {
		exit(0);
	}

	//產生第二個消除salt noise的圖片(result_salt)，Q帶入Q2=-1.5
	apply_contraharmonic_filter(data1, data2, biWidth, biHeight, Q2);

	if ((output_file = fopen("result_salt.bmp", "wb")) == NULL) {
		fprintf(stderr, "Output file can't open.\n");
		exit(0);
	}
	fwrite(bmpfileheader, sizeof(bmpfileheader), 1, output_file);
	fwrite(bmpinfoheader, sizeof(bmpinfoheader), 1, output_file);
	fwrite(color_table, 1024, 1, output_file);
	fwrite(data2, ((biWidth * 1 + 3) / 4 * 4) * biHeight * 1, 1, output_file);
	fclose(output_file);

	free(data1);
	free(data2);

	return 0;
}

void correctedPos(int* new_i, int* new_j, int i, int j, int M, int N)
{
	*new_i = i;
	*new_j = j;
	if (i >= 0 && i < M && j >= 0 && j < N)
		return;

	if (i < 0)
		*new_i = 0;
	else if (i >= M)
		*new_i = M - 1;

	if (j < 0)
		*new_j = 0;
	else if (j >= N)
		*new_j = N - 1;
}

int ReadDataSize(char* name)
{
	FILE* input_file = 0;
	U_CHAR bmpfileheader[14] = { 0 };
	U_CHAR bmpinfoheader[40] = { 0 };

	INT32 biWidth = 0;
	INT32 biHeight = 0;
	INT16 BitCount = 0;

	if ((input_file = fopen(name, "rb")) == NULL) {
		fprintf(stderr, "File can't open.\n");
		exit(0);
	}

	FREAD(input_file, bmpfileheader, 14);
	FREAD(input_file, bmpinfoheader, 40);

	if (GET_2B(bmpfileheader, 0) == 0x4D42)
		fprintf(stdout, "BMP file.\n");
	else {
		fprintf(stdout, "Not bmp file.\n");
		exit(0);
	}

	biWidth = GET_4B(bmpinfoheader, 4);
	biHeight = GET_4B(bmpinfoheader, 8);
	BitCount = GET_2B(bmpinfoheader, 14);

	if (BitCount != 8) {
		fprintf(stderr, "Not a 8-bit file.\n");
		fclose(input_file);
		exit(0);
	}

	fclose(input_file);

	return ((biWidth * 1 + 3) / 4 * 4) * biHeight * 1;
}

void ReadImageData(char* name, U_CHAR* bmpfileheader, U_CHAR* bmpinfoheader, U_CHAR* color_table, U_CHAR* data)
{
	FILE* input_file = 0;

	if ((input_file = fopen(name, "rb")) == NULL) {
		fprintf(stderr, "File can't open.\n");
		exit(0);
	}

	FREAD(input_file, bmpfileheader, 14);
	FREAD(input_file, bmpinfoheader, 40);

	if (GET_2B(bmpfileheader, 0) == 0x4D42)
		fprintf(stdout, "BMP file.\n");
	else {
		fprintf(stdout, "Not bmp file.\n");
		exit(0);
	}

	FREAD(input_file, color_table, 1024);
	fseek(input_file, GET_4B(bmpfileheader, 10), SEEK_SET);
	FREAD(input_file, data, ((GET_4B(bmpinfoheader, 4) * 1 + 3) / 4 * 4) * GET_4B(bmpinfoheader, 8) * 1);
	fclose(input_file);
}


void apply_contraharmonic_filter(U_CHAR* input, U_CHAR* output, int width, int height, double Q) {
	int biWidth4 = ((width * 1 + 3) / 4 * 4);
	int a = (SIZE - 1) / 2;
	double numerator, denominator;//分子numerator，分母denominator

	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			numerator = 0.0;
			denominator = 0.0;
			for (int i1 = i - a; i1 <= i + a; i1++) {
				for (int j1 = j - a; j1 <= j + a; j1++) {
					int ni, nj;
					correctedPos(&ni, &nj, i1, j1, height, width);
					double pixel = (double)input[ni * biWidth4 + nj];//向素存入pixel
					numerator += pow(pixel, Q + 1);//加總 g(s,t)^(Q+1)
					denominator += pow(pixel, Q);  //加總 g(s,t)^(Q)
				}
			}
			//sigma( g(s,t) ^ (Q+1) )
			//-----------------------
			//sigma( g(s,t) ^ (Q)   )
			output[i * biWidth4 + j] = (U_CHAR)(numerator / denominator);//相除
			
		}
	}
}
