#include<stdio.h>
#include<stdlib.h>
#pragma warning (disable:4996)

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

#define RATIO 2
#define SIZE	3//預設為3
void set_2B(U_CHAR* array, int offset, INT16 value);
void set_4B(U_CHAR* array, int offset, INT32 value);

int ReadDataSize(const char* name);
void ReadImageData(const char* name, U_CHAR* bmpfileheader, U_CHAR* bmpinfoheader, U_CHAR* color_table, U_CHAR* data);
void correctedPos(int* new_i, int* new_j, int i, int j, int M, int N);

int main()
{
    FILE* output_file = 0;

    U_CHAR bmpfileheader1[14] = { 0 };
    U_CHAR bmpinfoheader1[40] = { 0 };
    U_CHAR* data1, * data2, * new_data, color_table1[1024];
    U_CHAR* data, color_table[1024];

    U_CHAR new_bmpfileheader[14] = { 0 };
    U_CHAR new_bmpinfoheader[40] = { 0 };

    INT32 biWidth = 0;
    INT32 biHeight = 0;


    int i, j, k, temp, biWidth4, sum, i1, j1, mi, mj, ni, nj;
    int p, newbiWidth4, newbiHeight;
    int histo_table[256] = { 0 };
    int dataArray[SIZE][SIZE];
    int a, size_square, idx_col;


    i = ReadDataSize("Fig0417(a).bmp");
    data1 = (U_CHAR*)malloc(i);
    if (data1 == NULL) {
        exit(0);
    }

    ReadImageData("Fig0417(a).bmp", bmpfileheader1, bmpinfoheader1, color_table1, data1);
    biWidth = GET_4B(bmpinfoheader1, 4);
    biHeight = GET_4B(bmpinfoheader1, 8);

    //
    //i = ReadDataSize("new2.bmp");
    data2 = (U_CHAR*)malloc(i);
    if (data2 == NULL) {
        exit(0);
    }
    new_data = (U_CHAR*)malloc(i);
    if (new_data == NULL) {
        exit(0);
    }

    biWidth4 = ((biWidth * 1 + 3) / 4 * 4);
    a = (SIZE - 1) / 2;
    size_square = SIZE * SIZE;

    newbiWidth4 = ((biWidth / RATIO + 3) / 4 * 4);
    newbiHeight = biHeight / RATIO;

  

     //average
     for (i = 0; i < biHeight; i++)
     {
         sum = 0;
         // fill the dataArray
         mi = 0;
         for (i1 = i - a; i1 <= i + a; i1++)
         {
             mj = 0;
             for (j1 = 0 - a; j1 <= a; j1++)
             {
                 correctedPos(&ni, &nj, i1, j1, biHeight, biWidth);
                 dataArray[mi][mj] = data1[ni * biWidth4 + nj];
                 sum = sum + dataArray[mi][mj];
                 mj++;
             }
             mi++;
         }
         k = i * biWidth4;
         data2[k] = sum / size_square;
         k = k + 1;
         //
         idx_col = 0;
         for (j = 1; j < biWidth; j++)
         {
             // update the dataArray
             // remove old data and add new data
             mi = 0;
             for (i1 = i - a; i1 <= i + a; i1++)
             {
                 correctedPos(&ni, &nj, i1, j, biHeight, biWidth);
                 sum = sum - dataArray[mi][idx_col];
                 dataArray[mi][idx_col] = data1[ni * biWidth4 + nj];
                 sum = sum + dataArray[mi][idx_col];
                 mi++;
             }
             data2[k] = sum / size_square;
             idx_col = (idx_col + 1) % SIZE;
             k = k + 1;
         }
     }

     new_data = (U_CHAR*)malloc(newbiWidth4 * newbiHeight);
     if (new_data == NULL) {
         fprintf(stderr, "Insufficient memory.\n");
         exit(0);
     }

     //50% 
     p = 0;
     for (i = 0; i < biHeight; i++)
     {
         if (i % RATIO == 0)
         {
             k = i * biWidth4;
             p = (i / RATIO) * newbiWidth4;
             for (j = 0; j < biWidth; j++)
             {
                 if (j % RATIO == 0)
                 {
                     new_data[p] = data2[k];
                     p = p + 1;
                 }
                 k = k + 1;
             }
         }
     }
     // new file header
     for (i = 0; i < 14; i++)
         new_bmpfileheader[i] = bmpfileheader1[i];
     for (i = 0; i < 40; i++)
         new_bmpinfoheader[i] = bmpinfoheader1[i];
     set_4B(new_bmpfileheader, 2, newbiWidth4 * newbiHeight + 1078);
     set_4B(new_bmpinfoheader, 4, biWidth / RATIO);
     set_4B(new_bmpinfoheader, 8, biHeight / RATIO);
     set_4B(new_bmpinfoheader, 20, newbiWidth4 * newbiHeight);
    //
    /* 開啟新檔案 */
    if ((output_file = fopen("average.bmp", "wb")) == NULL) {
        fprintf(stderr, "Output file can't open.\n");
        exit(0);
    }

    fwrite(new_bmpfileheader, sizeof(bmpfileheader1), 1, output_file);
    fwrite(new_bmpinfoheader, sizeof(bmpinfoheader1), 1, output_file);

    fwrite(color_table1, 1024, 1, output_file);

    fwrite(new_data, newbiWidth4* newbiHeight, 1, output_file);

    fclose(output_file);

    //free(data1);

    //   free(data3);//new

    return 0;
}

int ReadDataSize(const char* name)
{
    FILE* input_file = 0;
    U_CHAR bmpfileheader[14] = { 0 };
    U_CHAR bmpinfoheader[40] = { 0 };

    INT32 biWidth = 0;
    INT32 biHeight = 0;
    INT16 BitCount = 0;

    /* 開啟檔案 */
    if ((input_file = fopen(name, "rb")) == NULL) {
        fprintf(stderr, "File can't open.\n");
        exit(0);
    }

    FREAD(input_file, bmpfileheader, 14);
    FREAD(input_file, bmpinfoheader, 40);

    if (GET_2B(bmpfileheader, 0) == 0x4D42) /* 'BM' */
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

    // 
    fclose(input_file);

    return ((biWidth * 1 + 3) / 4 * 4) * biHeight * 1;
}

void ReadImageData(const char* name, U_CHAR* bmpfileheader, U_CHAR* bmpinfoheader, U_CHAR* color_table, U_CHAR* data)
{
    FILE* input_file = 0;

    INT32 FileSize = 0;
    INT32 bfOffBits = 0;
    INT32 headerSize = 0;
    INT32 biWidth = 0;
    INT32 biHeight = 0;
    INT16 biPlanes = 0;
    INT16 BitCount = 0;
    INT32 biCompression = 0;
    INT32 biImageSize = 0;
    INT32 biXPelsPerMeter = 0, biYPelsPerMeter = 0;
    INT32 biClrUsed = 0;
    INT32 biClrImp = 0;

    /* 開啟檔案 */
    if ((input_file = fopen(name, "rb")) == NULL) {
        fprintf(stderr, "File can't open.\n");
        exit(0);
    }

    FREAD(input_file, bmpfileheader, 14);
    FREAD(input_file, bmpinfoheader, 40);

    if (GET_2B(bmpfileheader, 0) == 0x4D42) /* 'BM' */
        fprintf(stdout, "BMP file.\n");
    else {
        fprintf(stdout, "Not bmp file.\n");
        exit(0);
    }

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

    printf("FileSize = %ld \n"
        "DataOffset = %ld \n"
        "HeaderSize = %ld \n"
        "Width = %ld \n"
        "Height = %ld \n"
        "Planes = %d \n"
        "BitCount = %d \n"
        "Compression = %ld \n"
        "ImageSize = %ld \n"
        "XpixelsPerM = %ld \n"
        "YpixelsPerM = %ld \n"
        "ColorsUsed = %ld \n"
        "ColorsImportant = %ld \n", FileSize, bfOffBits, headerSize, biWidth, biHeight, biPlanes,
        BitCount, biCompression, biImageSize, biXPelsPerMeter, biYPelsPerMeter, biClrUsed, biClrImp);

    if (BitCount != 8) {
        fprintf(stderr, "Not a 8-bit file.\n");
        fclose(input_file);
        exit(0);
    }

    FREAD(input_file, color_table, 1024);

    //

    //
    fseek(input_file, bfOffBits, SEEK_SET);
    FREAD(input_file, data, ((biWidth * 1 + 3) / 4 * 4) * biHeight * 1);
    // 
    fclose(input_file);
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
void set_2B(U_CHAR* array, int offset, INT16 value)
{
    array[offset] = (U_CHAR)(value % 256);
    array[offset + 1] = (U_CHAR)(value >> 8);
}

void set_4B(U_CHAR* array, int offset, INT32 value)
{
    INT32 i;
    i = value;
    array[offset] = (U_CHAR)(i % 256);
    i = i >> 8;
    array[offset + 1] = (U_CHAR)(i % 256);
    i = i >> 8;
    array[offset + 2] = (U_CHAR)(i % 256);
    i = i >> 8;
    array[offset + 3] = (U_CHAR)(i % 256);
}
