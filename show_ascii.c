#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void put_ascii()
{
	FILE * ap;
	if((ap=fopen("ASC0814.dat", "rb")) == NULL)  
	{  
		printf("Can't Open ASC\n");  
		getchar(); 
		return 0; 
	}  
	char mat[14];
	int d;
	d = 'L';
	fseek(ap, 14*d, SEEK_SET);  
	fread(mat, 14, 1, ap); 

	int i, b;
	unsigned char byte;
 
	for (i = 0; i < 14; i++)//点阵有16行
	{
		byte = mat[i];
		for (b = 0;b<7;b++)
		{
			if (byte & (1<<b))//判断点阵中的各个点是否为1
			{
				/* show */
				printf("@");
			}
			else
			{
				/* hide */
				printf("-");
			}
		}
		printf("\n");
	}

}


int main()
{
	
	put_ascii();
	
	
	
	
	return 0;
}