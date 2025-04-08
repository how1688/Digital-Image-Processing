#include<stdio.h>
#include<stdlib.h>

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

void set_2B(U_CHAR *array, int offset, INT16 value);
void set_4B(U_CHAR *array, int offset, INT32 value);
U_CHAR bilinearInterpolation(U_CHAR *src, int width, int height, float x, float y);

int main()
{
   FILE *input_file = 0 ;
   FILE *output_file = 0 ;
   
   U_CHAR bmpfileheader[14] = { 0 } ;
   U_CHAR bmpinfoheader[40] = { 0 } ;

   U_CHAR new_bmpfileheader[14] = { 0 } ;
   U_CHAR new_bmpinfoheader[40] = { 0 } ;

   INT32 FileSize = 0 ;
   INT32 bfOffBits =0 ;
   INT32 headerSize =0 ;
   INT32 biWidth = 0 ;		
   INT32 biHeight = 0 ;
   INT16 biPlanes = 0 ;
   INT16 BitCount = 0 ;
   INT32 biCompression = 0 ;
   INT32 biImageSize = 0;
   INT32 biXPelsPerMeter = 0 ,biYPelsPerMeter = 0 ;
   INT32 biClrUsed = 0 ;
   INT32 biClrImp = 0 ;

   U_CHAR *data, *new_data, color_table[1024];
   float x_ratio, y_ratio;
   int i, j, k, p;
   INT32 newWidth, newHeight;

   /* 開啟檔案 */
   if( ( input_file = fopen("shrink.bmp","rb") ) == NULL ){
      fprintf(stderr,"File can't open.\n");
      exit(0);
   }


   FREAD(input_file,bmpfileheader,14);
   FREAD(input_file,bmpinfoheader,40);

   if (GET_2B(bmpfileheader,0) == 0x4D42) /* 'BM' */
      fprintf(stdout,"BMP file.\n");
   else{
      fprintf(stdout,"Not bmp file.\n");
      exit(0);
   }

   FileSize           =   GET_4B(bmpfileheader,2);
   bfOffBits         =   GET_4B(bmpfileheader,10);
   headerSize      =   GET_4B(bmpinfoheader,0);
   biWidth           =   GET_4B(bmpinfoheader,4);
   biHeight          =   GET_4B(bmpinfoheader,8);
   biPlanes          =   GET_2B(bmpinfoheader,12);
   BitCount          =   GET_2B(bmpinfoheader,14);
   biCompression   =   GET_4B(bmpinfoheader,16);
   biImageSize      =   GET_4B(bmpinfoheader,20);
   biXPelsPerMeter =   GET_4B(bmpinfoheader,24);
   biYPelsPerMeter =   GET_4B(bmpinfoheader,28);
   biClrUsed         =   GET_4B(bmpinfoheader,32);
   biClrImp          =   GET_4B(bmpinfoheader,36);

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
	"ColorsImportant = %ld \n",FileSize,bfOffBits,headerSize,biWidth,biHeight,biPlanes,
	BitCount,biCompression,biImageSize,biXPelsPerMeter,biYPelsPerMeter,biClrUsed,biClrImp);

   if (BitCount != 8) {
      fprintf(stderr,"Not a 8-bit file.\n");
      fclose (input_file);
      exit(0);
   }
	
   printf("原始影像尺寸: %ld x %ld\n", biWidth, biHeight);
    
   // 讓使用者輸入新影像大小
   printf("輸入新的影像寬度: ");
   scanf("%d", &newWidth);
    printf("輸入新的影像高度: ");
    scanf("%d", &newHeight);

    // 計算縮放比例
    x_ratio = (float)biWidth / newWidth;
    y_ratio = (float)biHeight / newHeight;
		
   FREAD(input_file,color_table,1024);

   //
   data = (U_CHAR *)malloc( biWidth*biHeight*1 );
   if (data == NULL) {
      fprintf(stderr,"Insufficient memory.\n");
      fclose (input_file);
      exit(0);
   }
   
   //
   fseek(input_file,bfOffBits,SEEK_SET);
   FREAD(input_file,data,biWidth*biHeight*1);
   // 
   fclose (input_file);

   //
   new_data = (U_CHAR *)malloc(newWidth * newHeight);
   if (new_data == NULL) {
      fprintf(stderr,"Insufficient memory.\n");
      exit(0);
   }

   // Process the file
    for (i = 0; i < newHeight; i++) {
        for (j = 0; j < newWidth; j++) {
            float x = j * x_ratio;
            float y = i * y_ratio;
            new_data[i * newWidth + j] = bilinearInterpolation(data, biWidth, biHeight, x, y);
        }
    }
      
   // new file header
   for (i=0; i < 14; i++)
		new_bmpfileheader[i] = bmpfileheader[i];
   for (i=0; i < 40; i++)
		new_bmpinfoheader[i] = bmpinfoheader[i];
   set_4B(new_bmpfileheader, 2, newWidth * newHeight + 1078);
   set_4B(new_bmpinfoheader, 4, newWidth);
   set_4B(new_bmpinfoheader, 8, newHeight);
   set_4B(new_bmpinfoheader, 20, newWidth * newHeight);


   //
   /* 開啟新檔案 */
   if( ( output_file = fopen("zoom.bmp","wb") ) == NULL ){
      fprintf(stderr,"Output file can't open.\n");
      exit(0);
   }

   fwrite(new_bmpfileheader, sizeof(bmpfileheader), 1, output_file);
   fwrite(new_bmpinfoheader, sizeof(bmpinfoheader), 1, output_file);

   fwrite(color_table, 1024, 1, output_file);
 
   fwrite(new_data, 1, newWidth * newHeight, output_file);
   fclose (output_file);
  
   return 0;
}

void set_2B(U_CHAR *array, int offset, INT16 value)
{
	array[offset] = (U_CHAR) (value % 256);
	array[offset+1] = (U_CHAR) (value >> 8);
}

void set_4B(U_CHAR *array, int offset, INT32 value)
{
	INT32 i;
	i = value;
	array[offset] = (U_CHAR) (i % 256);
	i = i >> 8;
	array[offset+1] = (U_CHAR) (i % 256);
	i = i >> 8;
	array[offset+2] = (U_CHAR) (i % 256);
	i = i >> 8;
	array[offset+3] = (U_CHAR) (i % 256);
}

// 雙線性插值函數
U_CHAR bilinearInterpolation(U_CHAR *src, int width, int height, float x, float y) {
    int x1 = (int)x;
    int y1 = (int)y;
    int x2 = x1 + 1;
    int y2 = y1 + 1;

    if (x1 >= width - 1) x2 = x1;
    if (y1 >= height - 1) y2 = y1;

    U_CHAR Q11 = src[y1 * width + x1];
    U_CHAR Q21 = src[y1 * width + x2];
    U_CHAR Q12 = src[y2 * width + x1];
    U_CHAR Q22 = src[y2 * width + x2];

    float dx = x - x1;
    float dy = y - y1;

    float R1 = (1 - dx) * Q11 + dx * Q21;
    float R2 = (1 - dx) * Q12 + dx * Q22;
    float P = (1 - dy) * R1 + dy * R2;

    return (U_CHAR)P;
}
