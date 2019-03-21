#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned short int uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long int uint64_t;

typedef struct {
    //uint16_t bfType;            //文件类型，必须为BM
    uint32_t bfSize;            //文件大小
    uint16_t bfReserved1;       //文件保留字
    uint16_t bfReserved2;       //文件保留字
    uint32_t bfOffBits;         //文件头的偏移量
} BMPFILESTRUCT;

typedef struct {
    uint32_t biSize;            //从这里开始到结构体结束总共的字节数
    uint32_t biWidth;           //BM的宽度
    uint32_t biHeight;          //BM的高度
    uint16_t biPlanes;          //目标设备的级别,必须为1
    uint16_t biBitCount;        //每个像素的位数.
                                //1(双色,8个像素占1个字节),4(16色,2个像素占1个字节),8(256色,1个像素占1个字节),16(高彩色),24(真彩色,1个像素占3个字节,按顺序分别为B,G,R；)
    uint32_t biCompression;     //BM压缩类型,0(不压缩),1(BI_RLE8压缩类型),2(BI_RLE4压缩类型)
    uint32_t biSizeImage;       //BM的大小，其中包括为了补齐行数是4的倍数而添加的空字节
    uint32_t biXPelsPerMeter;   //BM水平分辨率，每米像素值
    uint32_t biYPelsPerMeter;   //BM垂直分辨率，每米像素值
    uint32_t biClrUsed;         //BM实际使用的颜色表中的颜色数
    uint32_t biClrImportant;    //BM显示过程中重要的颜色数
    
} BMPINFOSTRUCT;

void MakeBmpFile (char * imagedata,uint64_t width,uint64_t height,const char * filename)
{
    BMPFILESTRUCT filestruct;
    //filestruct.bfType = 0x4d42;  //'BM'
    filestruct.bfSize = width*height*3+sizeof(BMPFILESTRUCT)+sizeof(BMPINFOSTRUCT);
    filestruct.bfReserved1 = 0;
    filestruct.bfReserved2 = 0;
    filestruct.bfOffBits = sizeof(BMPFILESTRUCT)+sizeof(BMPINFOSTRUCT);

    BMPINFOSTRUCT infostruct;
    infostruct.biSize = sizeof(BMPINFOSTRUCT);
    infostruct.biWidth = width;
    infostruct.biHeight = height;
    infostruct.biPlanes = 1;
    infostruct.biBitCount = 24;
    infostruct.biCompression = 0;
    infostruct.biSizeImage = width*height*3;
    infostruct.biXPelsPerMeter = 0;
    infostruct.biYPelsPerMeter = 0;
    infostruct.biClrUsed = 0;
    infostruct.biClrImportant = 0;

    FILE *fp=fopen(filename,"wb");
    if(!fp) {
        printf("open file failed!");
        return;
    }
    fwrite("BM",1,2,fp);
    int ret=fwrite(&filestruct,1,sizeof(BMPFILESTRUCT),fp);
    printf("ret = %d",ret);
    ret=fwrite(&infostruct,1,sizeof(BMPINFOSTRUCT),fp);
	printf("ret = %d",ret);
	
	fclose(fp);

}

int hanzi16(unsigned char incode[3])
{ 
    typedef struct RGBimagedata{
		char r;
		char g;
		char b;

	}RGBimagedata;
	RGBimagedata imagedata;
	imagedata.b = 255;
	imagedata.g = 100;
	imagedata.r = 97;
	
	RGBimagedata imagedata1;
	imagedata1.b = 255;
	imagedata1.g = 0;
	imagedata1.r = 0;
	
	//printf("%x	%x	%x\n",incode[0],incode[1],incode[2]);
	unsigned char qh = 0, wh = 0;  
	unsigned long offset = 0;  
	char mat[16][2] = {0};  
	FILE *HZK = 0;  
	int i,j,k;  

	//每个汉字，占两个字节, 取其区位号  
	qh = incode[0]- 0xa0;	//获得区码  
	wh = incode[1]- 0xa0;	//获得位码  
	//printf("%x	%x\n",qh,wh);
	offset = (94*(qh-1)+(wh-1))*32; //得到偏移位置 
	if((HZK=fopen("HZK1616.dat", "rb")) == NULL)  
	{  
		printf("Can't Open HZK16\n");  
		getchar(); 
		return 0; 
	}  
	fseek(HZK, offset, SEEK_SET);  
	fread(mat, 32, 1, HZK); 

    
	//显示 
	MakeBmpFile ((char *)&imagedata,16,16,"red.bmp");
	FILE *fp=fopen(filename,"wb");
	 if(fp == NULL)
	 {
		printf("Can't Open bmpfile\n");  
		getchar(); 
		return 0; 
		 
	 }
	fseek(fp, 52, SEEK_SET);  
	for(i=0; i<16; i++) 
	{	
		for(j=0; j<2; j++) 
		{ 
			for(k=0; k<8; k++) 
			{ 
				if(mat[i][j] & (0x80>>k))  
				{
				ret=fwrite(imagedata,1,3,fp);
				} 
				else 
				{ 
				ret=fwrite(imagedata1,1,3,fp);
				}	
			}	
		} 
		printf("\n"); 
	} 

	fclose(HZK);
	fclose(fp);
	return 0; 
	
}

int main()
{
	
	
  unsigned char incode[3]= "入";
  hanzi16(incode);

  return 0;

}
	

	
	
	
	
	
	
