#include<stdio.h>
#include<stdlib.h>
#include<math.h>

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

int ReadDataSize(char *name);
void ReadImageData(char *name, U_CHAR *bmpfileheader, U_CHAR *bmpinfoheader, U_CHAR *color_table, U_CHAR *data);

int main()
{
   FILE *output_file = 0 ;
   
   U_CHAR bmpfileheader1[14] = { 0 } ;
   U_CHAR bmpinfoheader1[40] = { 0 } ;
   U_CHAR *data1, *data2, *new_data, color_table1[1024];

   INT32 biWidth = 0 ;		
   INT32 biHeight = 0 ;

   int i, j, k, temp, l;
   int histo_table[256] = { 0 };

   i = ReadDataSize("Fig2.24(a).bmp");
   data1 = (U_CHAR *)malloc( i );
   if (data1 == NULL) {
      exit(0);
   }

   ReadImageData("Fig2.24(a).bmp", bmpfileheader1, bmpinfoheader1, color_table1, data1);
   biWidth           =   GET_4B(bmpinfoheader1,4);
   biHeight          =   GET_4B(bmpinfoheader1,8);

   //
   //i = ReadDataSize("new2.bmp");
   data2 = (U_CHAR *)malloc( i );
   if (data2 == NULL) {
      exit(0);
   }

   char filename[20];
   // Process the file
   for (i=1; i < 8; i++){ // 階層數
	  int level = pow(2, i); // 2^i
      for (j=0; j < biHeight; j++){
         for(k=0; k<=biWidth; k++){
            int interval = level - 1; // 根據階層分區間
            int a[128] = {0}; // 存放區間值
            for(l=0; l<level; l++){
               a[l] = 255/interval*l;
            }
            for(l=0; l<level-1; l++){
               if(data1[j*biWidth + k] >= a[l] && data1[j*biWidth + k] < a[l+1]){
                  if(abs(a[l]-data1[j*biWidth + k]) > abs(a[l+1]-data1[j*biWidth + k])){
                     data2[j*biWidth + k] = a[l+1];
                  }
                  else{
                     data2[j*biWidth + k] = a[l];
                  }
                  break;
               }
            }
         }
      }
      // 產生檔案名稱
      sprintf(filename, "level_%d.bmp", level);

      // 開啟輸出檔案
      if ((output_file = fopen(filename, "wb")) == NULL) {
         fprintf(stderr, "Output file %s can't open.\n", filename);
         exit(0);
      }

      fwrite(bmpfileheader1, sizeof(bmpfileheader1), 1, output_file);
      fwrite(bmpinfoheader1, sizeof(bmpinfoheader1), 1, output_file);

      fwrite(color_table1, 1024, 1, output_file);
      fwrite(data2, ((biWidth*1 +3)/4 *4)*biHeight*1, 1, output_file); 
      fclose (output_file);
   }
   
   free(data1);
   free(data2);

   return 0;
}

int ReadDataSize(char *name)
{
   FILE *input_file = 0 ;
   U_CHAR bmpfileheader[14] = { 0 } ;
   U_CHAR bmpinfoheader[40] = { 0 } ;
   
   INT32 biWidth = 0 ;		
   INT32 biHeight = 0 ;
   INT16 BitCount = 0 ;

   /* 嚙罷嚙踝蕭嚙褕殷蕭 */
   if( ( input_file = fopen(name,"rb") ) == NULL ){
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

   biWidth           =   GET_4B(bmpinfoheader,4);
   biHeight          =   GET_4B(bmpinfoheader,8);
   BitCount          =   GET_2B(bmpinfoheader,14);

   if (BitCount != 8) {
      fprintf(stderr,"Not a 8-bit file.\n");
      fclose (input_file);
      exit(0);
   }

   // 
   fclose (input_file);

   return ((biWidth*1 +3)/4 *4)*biHeight*1;
}

void ReadImageData(char *name, U_CHAR *bmpfileheader, U_CHAR *bmpinfoheader, U_CHAR *color_table, U_CHAR *data)
{
   FILE *input_file = 0 ;
   
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

   /* 嚙罷嚙踝蕭嚙褕殷蕭 */
   if( ( input_file = fopen(name,"rb") ) == NULL ){
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

   FREAD(input_file,color_table,1024);

   //
   
   //
   fseek(input_file,bfOffBits,SEEK_SET);
   FREAD(input_file,data,((biWidth*1 +3)/4 *4)*biHeight*1);
   // 
   fclose (input_file);
}
