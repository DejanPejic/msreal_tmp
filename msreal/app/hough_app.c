#include "app_functions.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../../esl/spec/bit_analysis/stb_image.hpp"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../../esl/spec/bit_analysis/stb_image_write.hpp"

int main(int argc, char *argv[])
{	
	float draw_factor;
	unsigned char *img;
	unsigned char *result_img;
	unsigned char *gray_img;
	unsigned char *edge_img;
	int width, height, channels;
	int16_t rho;
	size_t gray_img_size;
	size_t img_size;
	unsigned char *acc0;
	unsigned char *acc1;
	size_t acc_size;
	unsigned char acc_max;
	int i, j;
	unsigned char result_image[] = "result_img/i.jpg";
    
	for (i = 0; i < argc-2; i+=2)
	{
		draw_factor = atof(argv[i+1]);
		img = stbi_load(argv[i+2], &width, &height, &channels, 0);
		if (img == NULL)
		{
			printf("Greska prilikom ucitavanja fotografije.\n");
			exit(1);
		}

		gray_img_size = width * height;
		img_size = gray_img_size * channels;
		
		rho = (int16_t) sqrt(width*width + height*height);
		if (rho > RHO)
		{
			printf("Uneli ste preveliku fotografiju (dijagonala = %d)!\n NAJVECA DOZVOLJENA VREDNOST JE %d\n)", rho, RHO);
			exit(1);
		}
		
		acc_size = THETA2*rho;

		result_img = (unsigned char *)malloc(img_size);
		if(result_img == NULL)
		{
			printf("Nije moguca alokacija memorije za rezultujucu fotografiju.\n");
			exit(1);
		}

		gray_img = (unsigned char *)malloc(gray_img_size);
		if(gray_img == NULL)
		{
			printf("Nije moguca alokacija memorije za monohromatsku fotografiju.\n");
			exit(1);
		}

		edge_img = (unsigned char *)malloc(gray_img_size);
		if(edge_img == NULL)
		{
			printf("Nije moguca alokacija memorije za fotografiju sa izdvojenim ivicama.\n");
			exit(1);
		}
		
		acc0 = (unsigned char *)malloc(acc_size);
		if(acc0 == NULL)
		{
			printf("Nije moguca alokacija memorije za akumulator.\n");
			exit(1);
		}
		
		acc1 = (unsigned char *)malloc(acc_size);
		if(acc1 == NULL)
		{
			printf("Nije moguca alokacija memorije za akumulator.\n");
			exit(1);
		}

		copying(img, result_img, img_size);
		monochromating(img, gray_img, img_size);
		edging(gray_img, edge_img, width, height, gray_img_size);

		acc_init(acc0, rho);
		acc_init(acc1, rho);
		
		//Umesto funkcije hough(edge_img, acc, width, height, THRESHOLD);
		//transformacija se izvrsaava u IP_Core-u
		
		write_hard(RESET_REG, 1);
		write_hard(WIDTH_REG, width); 
		write_hard(HEIGHT_REG, height);
		write_hard(RHO_REG, rho);
		write_hard(THRESHOLD_REG, THRESHOLD);
		//printf("Zavrsena inicijalizacija od strane softvera\n");
		
		write_bram(ACC0_BRAM, acc0, acc_size);
		write_bram(ACC1_BRAM, acc1, acc_size);
		write_bram(IMG_BRAM, edge_img, gray_img_size);
		//printf("BRAM-ovi napunjeni podacima\n");
		
		//read_bram(ACC0_BRAM, acc0, THETA2*rho);
		//read_bram(ACC1_BRAM, acc1, THETA2*rho);
		//read_bram(IMG_BRAM, gray_img, width*height);
		
		write_hard(START_REG, 1);
		
		while(!read_hard(READY_REG));
		
		//printf("Zavrsena transformacija u hardveru!\n");
		
		read_bram(ACC0_BRAM, acc0, THETA2*rho);
		read_bram(ACC1_BRAM, acc1, THETA2*rho);
		
		//printf("Ocitan akumulator\n");
		
		// nastavak softverskog dela
	
		acc_max = max_acc(acc0, acc1, rho);
		
		/*
		printf("\nFOTOGRAFIJA:");
                for(j = 0; j < width*height; j++)
                {
                        printf("img[%d]=%d, ", j, gray_img[j]);
                        if((j + 1) % 5 == 0) printf("\n");
                }
		
		printf("\nAKUMULATOR0:");	
		for(j = 0; j < THETA2*rho; j++)
		{
			printf("acc0[%d]=%d, ", j, acc0[j]);
			if((j + 1) % 5 == 0) printf("\n");
		}
		
		printf("\nAKUMULATOR1:");
		for(j = 0; j < THETA2*rho; j++)
                {
                        printf("acc1[%d]=%d, ", j, acc1[j]);
                        if((j + 1) % 5 == 0) printf("\n");
                }
		
		printf("\n\nacc0[%d] = %d\n\n", 0, acc0[0]);
		printf("acc0[%d] = %d\n", 452, acc0[452]);
		printf("acc0[%d] = %d\n\n", 1578, acc0[1578]);

		printf("acc1[%d] = %d\n", 1014, acc1[1014]);
		printf("acc1[%d] = %d\n", 330, acc1[330]);

		printf("maksimalna vrednost akumulatora: %d\n", acc_max);
		*/
		
		draw_lines(result_img, acc0, acc1, rho, width, height, acc_max, draw_factor);

		result_image[11] = ((i/2) % 10) + 48;
		if (i%20 == 0) result_image[10]++;
		stbi_write_jpg((const char*) result_image, width, height, channels, result_img, 100);

		free(result_img);
		free(gray_img);
		free(edge_img);
		free(acc0);
		free(acc1);
		stbi_image_free(img);
		printf("image%d done!\n", i/2);
	}
	
	printf("End of processing!\n");
	return 0;
}
