

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
	
	int i;
	for(i=0;i<width*height*4;i++)
    ret=fwrite(imagedata,1,3,fp);

	printf("ret = %d",ret);

	fclose(fp);

}
