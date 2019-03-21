#include <stdio.h>
#include <stdlib.h>
#include "bmp.h"

int main(void)
{
 
  /*  
    int i,j;
	struct {
        char b;
        char g;
        char r;
    } imagedata[800][480];
    printf("%ld",sizeof(imagedata));
    for(i=0 ; i < 800 ; i++) 
	for(j=0 ; j < 480 ;j++) {
        	imagedata[i][j].b = 1;
        	imagedata[i][j].g = 1;
       	 	imagedata[i][j].r = 255;
    	} */
	typedef struct RGBimagedata{
		char r;
		char g;
		char b;

	}RGBimagedata;
	RGBimagedata imagedata;
	imagedata.b = 255;
	imagedata.g = 100;
	imagedata.r = 97;
    MakeBmpFile ((char *)&imagedata,480,480,"red.bmp");
    return 0;
}
