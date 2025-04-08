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

#define RATIO 4

void set_2B(U_CHAR *array, int offset, INT16 value);
void set_4B(U_CHAR *array, int offset, INT32 value);


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
   int i, j, k, p;

   /* 開啟檔案 */
   if( ( input_file = fopen("Fig2.19(a).bmp","rb") ) == NULL ){
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
   new_data = (U_CHAR *)malloc( biWidth*biHeight*1/RATIO/RATIO );
   if (new_data == NULL) {
      fprintf(stderr,"Insufficient memory.\n");
      exit(0);
   }

   // Process the file
   p = 0;
   for (i=0; i < biHeight; i++)
   {
       if (i%RATIO == 0)
	   {
			k = i* ((biWidth*1 +3)/4 *4);
			for (j=0; j < biWidth; j++)
			{
				if (j%RATIO == 0)
				{
					new_data[p] = data[k];
					p = p+1;
				}
	            k = k+1;
			}
       }
   }
      
   // new file header
   for (i=0; i < 14; i++)
		new_bmpfileheader[i] = bmpfileheader[i];
   for (i=0; i < 40; i++)
		new_bmpinfoheader[i] = bmpinfoheader[i];
   set_4B(new_bmpfileheader, 2, biWidth*biHeight/RATIO/RATIO + 1078);
   set_4B(new_bmpinfoheader, 4, biWidth/RATIO);
   set_4B(new_bmpinfoheader, 8, biHeight/RATIO);
   set_4B(new_bmpinfoheader, 20, biWidth*biHeight/RATIO/RATIO);


   //
   /* 開啟新檔案 */
   if( ( output_file = fopen("new.bmp","wb") ) == NULL ){
      fprintf(stderr,"Output file can't open.\n");
      exit(0);
   }

   fwrite(new_bmpfileheader, sizeof(bmpfileheader), 1, output_file);
   fwrite(new_bmpinfoheader, sizeof(bmpinfoheader), 1, output_file);

   fwrite(color_table, 1024, 1, output_file);
 
   fwrite(new_data, biWidth*biHeight*1/RATIO/RATIO, 1, output_file);
 
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
