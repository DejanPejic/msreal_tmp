#include <stdio.h>
#include <stdlib.h>

#define RHO 442
#define HALF_THETA 135

void monochromating(unsigned char *img, unsigned char *gray_img, size_t img_size);
void edging(unsigned char *img, unsigned char *edge_img, int width, int height, size_t img_size);
	

int main(int argc, char *argv[])
{	
	int rho;
	int val;
	size_t acc_size;
	int i;
	
	unsigned char *acc0_vp;
	unsigned char *acc1_vp;
	unsigned char *acc0_msreal;
	unsigned char *acc1_msreal;
	
	FILE *acc0_vp_fp;
	FILE *acc1_vp_fp;
	FILE *acc0_msreal_fp;
	FILE *acc1_msreal_fp;
	FILE *dim_fp;
	
	char acc0_vp_n[] = "acc0_vp.txt";
	char acc1_vp_n[] = "acc1_vp.txt";
	char acc0_msreal_n[] = "acc0_msreal.txt";
	char acc1_msreal_n[] = "acc1_msreal.txt";
	char dim_file[] = "dim_file.txt";
	
	int acc0_diff = 0;
	int acc1_diff = 0;
	
	dim_fp = fopen((const char*) dim_file, "r");
	if (dim_fp == NULL)
	{
		printf("Greska prilikom otvaranja fajla acc1!");
		exit(1);
	}
	fscanf(dim_fp, "%d\n", &rho);
	fclose(dim_fp);
	
    	printf("rho = %d\n", rho);
    	acc_size = rho * HALF_THETA;
    	
    	acc0_vp = (unsigned char *)malloc(acc_size);
	if(acc0_vp == NULL)
	{
		printf("Nije moguca alokacija memorije za akumulator.\n");
		exit(1);
	}
	
	acc1_vp = (unsigned char *)malloc(acc_size);
	if(acc1_vp == NULL)
	{
		printf("Nije moguca alokacija memorije za akumulator.\n");
		exit(1);
	}
	
	acc0_msreal = (unsigned char *)malloc(acc_size);
	if(acc0_msreal == NULL)
	{
		printf("Nije moguca alokacija memorije za akumulator.\n");
		exit(1);
	}
	
	acc1_msreal = (unsigned char *)malloc(acc_size);
	if(acc1_msreal == NULL)
	{
		printf("Nije moguca alokacija memorije za akumulator.\n");
		exit(1);
	}
    	
    	acc0_vp_fp = fopen(acc0_vp_n, "r");
    	for (i = 0; i < acc_size; i++)
    	{
    		fscanf(acc0_vp_fp, "%d", &val);
    		acc0_vp[i] = val;
    	}
    	fclose(acc0_vp_fp);
    	
    	acc1_vp_fp = fopen(acc1_vp_n, "r");
    	for (i = 0; i < acc_size; i++)
    	{
    		fscanf(acc1_vp_fp, "%d", &val);
    		acc1_vp[i] = val;
    	}
    	fclose(acc1_vp_fp);
    	
    	acc0_msreal_fp = fopen(acc0_msreal_n, "r");
    	for (i = 0; i < acc_size; i++)
    	{
    		fscanf(acc0_msreal_fp, "%d", &val);
    		acc0_msreal[i] = val;
    	}
    	fclose(acc0_msreal_fp);
    	
    	acc1_msreal_fp = fopen(acc1_msreal_n, "r");
    	for (i = 0; i < acc_size; i++)
    	{
    		fscanf(acc1_msreal_fp, "%d", &val);
    		acc1_msreal[i] = val;
    	}
    	fclose(acc1_msreal_fp);
    	
    	/*
	for(i = 0; i < acc_size; i++)
	{
		printf("acc0_vp[%d]=%d, ", i, acc0_vp[i]);
		if((i + 1) % 5 == 0) printf("\n");
	}
	printf("\n\n");
    	
	for(i = 0; i < acc_size; i++)
	{
		printf("acc1_vp[%d]=%d, ", i, acc1_vp[i]);
		if((i + 1) % 5 == 0) printf("\n");
	}
	printf("\n\n");
    	
	for(i = 0; i < acc_size; i++)
	{
		printf("acc0_msreal[%d]=%d, ", i, acc0_msreal[i]);
		if((i + 1) % 5 == 0) printf("\n");
	}
	printf("\n\n");
    	
	for(i = 0; i < acc_size; i++)
	{
		printf("acc1_msreal[%d]=%d, ", i, acc1_msreal[i]);
		if((i + 1) % 5 == 0) printf("\n");
	}
	printf("\n\n");
	*/
	
	for(i = 0; i < acc_size; i++)
	{
		if (acc0_vp[i] != acc0_msreal[i])
		{
			printf("acc0 razlika: ");
			printf("acc0_vp[%d] = %d, a acc0_msreal[%d] = %d\n", i, acc0_vp[i], i, acc0_msreal[i]);
			acc0_diff++;
		}
	}
    	
    	for(i = 0; i < acc_size; i++)
	{
		if (acc1_vp[i] != acc1_msreal[i])
		{
			//printf("acc1 razlika: ");
			//printf("acc1_vp[%d] = %d, a acc1_msreal[%d] = %d\n", i, acc1_vp[i], i, acc1_msreal[i]);
			acc1_diff++;
		}
	}
	
	if (acc0_diff == 0 && acc1_diff == 0)
	{
    		printf("acc1_vp i acc1_msreal se podudaraju!\n");
    		printf("Bravo embedare!!!\n\n");
    	}
    	else
    	{
    		printf("Broj acc0 gresaka je %d.\n", acc0_diff);
    		printf("Broj acc1 gresaka je %d.\n", acc1_diff);
    	}
	
	return 0;
}

