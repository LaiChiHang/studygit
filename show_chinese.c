#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned short int uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long int uint64_t;

typedef struct {
    //uint16_t bfType;            //�ļ����ͣ�����ΪBM
    uint32_t bfSize;            //�ļ���С
    uint16_t bfReserved1;       //�ļ�������
    uint16_t bfReserved2;       //�ļ�������
    uint32_t bfOffBits;         //�ļ�ͷ��ƫ����
} BMPFILESTRUCT;

typedef struct RGBimagedata{
		char r;
		char g;
		char b;
		
	}RGBimagedata;	

typedef struct {
    uint32_t biSize;            //�����￪ʼ���ṹ������ܹ����ֽ���
    uint32_t biWidth;           //BM�Ŀ��
    uint32_t biHeight;          //BM�ĸ߶�
    uint16_t biPlanes;          //Ŀ���豸�ļ���,����Ϊ1
    uint16_t biBitCount;        //ÿ�����ص�λ��.
                                //1(˫ɫ,8������ռ1���ֽ�),4(16ɫ,2������ռ1���ֽ�),8(256ɫ,1������ռ1���ֽ�),16(�߲�ɫ),24(���ɫ,1������ռ3���ֽ�,��˳��ֱ�ΪB,G,R��)
    uint32_t biCompression;     //BMѹ������,0(��ѹ��),1(BI_RLE8ѹ������),2(BI_RLE4ѹ������)
    uint32_t biSizeImage;       //BM�Ĵ�С�����а���Ϊ�˲���������4�ı�������ӵĿ��ֽ�
    uint32_t biXPelsPerMeter;   //BMˮƽ�ֱ��ʣ�ÿ������ֵ
    uint32_t biYPelsPerMeter;   //BM��ֱ�ֱ��ʣ�ÿ������ֵ
    uint32_t biClrUsed;         //BMʵ��ʹ�õ���ɫ���е���ɫ��
    uint32_t biClrImportant;    //BM��ʾ��������Ҫ����ɫ��
    
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
	/* int i;
	for( i=0;i<16*16*4;i++)
	fwrite(&imagedata,1,3,fp);  */

	fclose(fp);

}

int hanzi16(unsigned char incode[3],unsigned char incode1[3],unsigned char incode2[3])
{ 
	RGBimagedata imagedata1;
	imagedata1.r = 200;
	imagedata1.g = 200;
	imagedata1.b = 200;

	//printf("%x	%x	%x\n",incode[0],incode[1],incode[2]);
	unsigned char qh = 0, wh = 0;  
	unsigned long offset = 0;  
	
	char mat[16][2] ;  
	char mat1[16][2] ;  
	char mat2[16][2] ;  
	FILE *HZK = 0;  
	int i,j,k;  

	//��ʾ 
	MakeBmpFile ((char *)&imagedata1,48,16,"red.bmp");
	FILE *fp=fopen("red.bmp","ab");
	 if(fp == NULL)
	 {
		printf("Can't Open bmpfile\n");  
		getchar(); 
		return 0; 
		 
	 } 
	//ÿ�����֣�ռ�����ֽ�, ȡ����λ��  
	qh = incode[0]- 0xa0;	//�������  
	wh = incode[1]- 0xa0;	//���λ��  
	//printf("%x	%x\n",qh,wh);
	offset = (94*(qh-1)+(wh-1))*32; //�õ�ƫ��λ�� 
	if((HZK=fopen("HZK1616.dat", "rb")) == NULL)  
	{  
		printf("Can't Open HZK16\n");  
		getchar(); 
		return 0; 
	}  
	fseek(HZK, offset, SEEK_SET);  
	fread(mat, 32, 1, HZK); 
	
	qh = incode1[0]- 0xa0;	//�������  
	wh = incode1[1]- 0xa0;	//���λ��  
	//printf("%x	%x\n",qh,wh);
	offset = (94*(qh-1)+(wh-1))*32; //�õ�ƫ��λ�� 
	fseek(HZK, offset, SEEK_SET); 
     fread(mat1, 32, 1, HZK); 	
	 
	qh = incode2[0]- 0xa0;	//�������  
	wh = incode2[1]- 0xa0;	//���λ��  
	offset = (94*(qh-1)+(wh-1))*32; //�õ�ƫ��λ�� 
	fseek(HZK, offset, SEEK_SET); 
     fread(mat2, 32, 1, HZK); 	
    
	 printf("\n"); 
	
	
	
	FILE * ap;
	if((ap=fopen("ASC0814.dat", "rb")) == NULL)  
	{  
		printf("Can't Open ASC\n");  
		getchar(); 
		return 0; 
	}  
	char asc[14];
	int d;
	d = 'L';
	fseek(ap, d*14, SEEK_SET);  
	fread(asc, 14, 1, ap); 

	int b;
	unsigned char byte;
 
	for (i = 0; i < 16; i++)//������16��
	{
		byte = asc[i];
		for (b = 0;b<16;b++)
		{
			if (byte & (1<<b))//�жϵ����еĸ������Ƿ�Ϊ1
			{
				printf("@");
			}
			else
			{
				printf("-");
			}
		}
		printf("\n");
	}
	
	for(i=0;i<16;i++)
	{
	RGBimagedata imagedata;
	imagedata.r = 255-i;
	imagedata.b = 15*i;
	imagedata.g = 10*i;
	
		for(j=0; j<2; j++) 
		{ 
				for(k=0; k<8; k++) 
				{ 	
					if(mat[15-i][j] & (0x80>>k))  
					 fwrite(&imagedata1,1,3,fp);	 
					else
					 fwrite(&imagedata,1,3,fp); 	
				}
		}		
		for(j=0; j<2; j++) 
		{ 
			for(k=0; k<8; k++) 
			{ 
				if(mat1[15-i][j] & (0x80>>k))  
				 fwrite(&imagedata1,1,3,fp);	
				else 
				 fwrite(&imagedata,1,3,fp); 
			}
		}
		for(j=0; j<2; j++) 
		{ 
			for(k=0; k<8; k++) 
			{ 
				if(mat2[15-i][j] & (0x80>>k))  
				 fwrite(&imagedata1,1,3,fp);
				else
				 fwrite(&imagedata,1,3,fp); 		
			}
		}
		
	}	
	fclose(HZK);
	fclose(fp);
	return 0; 
	
}

int main(int argc,char *argv[])
{
  unsigned char incode[3]  = "Ƕ";
  unsigned char incode1[3] = "��";
  unsigned char incode2[3] = "ʽ";
  
  hanzi16(incode,incode1,incode2);
  
  return 0;
}
	

	
	
	
	
	
	
