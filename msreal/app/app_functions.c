#include "app_functions.h"

void write_bram(int bram_n, unsigned char * val, int length)
{
	FILE *bram_file;
	int i;
	
	switch (bram_n)
	{
		case ACC0_BRAM:
			bram_file = fopen("/dev/acc0_bram_ctrl", "w");
			break;
		
		case ACC1_BRAM:
			bram_file = fopen("/dev/acc1_bram_ctrl", "w");
			break;
		
		case IMG_BRAM:
			bram_file = fopen("/dev/img_bram_ctrl", "w");
			break;
		
		default:
			printf("Wrong location for writing!\n");
			return;
	}
	
	fflush(bram_file);
	for (i = 0; i < length; i++)
	{
		fprintf(bram_file, "%d, %d\n", (int) val[i], i);
		fflush(bram_file);
		//printf("w:br%d[%d]=%d, ", bram_n, i, val[i]);
		//if((i + 1) % 5 == 0)
		//	printf("\n");
	}
	
	fclose(bram_file);
}

void read_bram(int bram_n, unsigned char *val, int length)
{
	FILE *bram_file;
	int tmp;
	int i;
	
	switch (bram_n)
	{
		case ACC0_BRAM:
			bram_file = fopen ("/dev/acc0_bram_ctrl", "r");
			break;
		
		case ACC1_BRAM:
			bram_file = fopen ("/dev/acc1_bram_ctrl", "r");
			break;	
		
		case IMG_BRAM:
			bram_file = fopen ("/dev/img_bram_ctrl", "r");
			break;
		
		default:
			printf("Wrong location for reading!\n");
			return;
	}

	for (i = 0; i < length; i++)
        {
		fscanf(bram_file, "%d ", &tmp);
                val[i] =  (unsigned char) tmp;
                //printf("r:br%d[%d]=%d, ", bram_n, i, val[i]);
                //if((i + 1) % 5 == 0)
                //      printf("\n");
        }
	fscanf(bram_file, "\n");
	fclose(bram_file);	
}

void write_hard(unsigned char addr, int val)
{
	FILE *hough_file;
	
	hough_file = fopen("/dev/hough_core", "w");
	fprintf(hough_file, "%d, %d\n", val, addr);
	fflush(hough_file);
	fclose(hough_file);
}

int read_hard(unsigned char addr)
{
	FILE *hough_file;
	int val;
	
	hough_file = fopen("/dev/hough_core", "r");
	fscanf(hough_file, "%d \n", &val);
	fclose(hough_file);
	
	return val;
}

void monochromating(unsigned char *img, unsigned char *gray_img, size_t img_size)
{
    unsigned char *p;
    unsigned char *pg;

    for(p = img, pg = gray_img; p != img + img_size; p += 3, pg++)
    {
        *pg = (uint8_t) ((*p + *(p + 1) + *(p + 2))/3.0);
    }
}

void copying(unsigned char *img, unsigned char *same_img, size_t img_size)
{
    unsigned char *p;
    unsigned char *pg;

    for(p = img, pg = same_img; p != img + img_size; p++, pg++)
    {
        *pg = *p;
    }
}

void edging(unsigned char *img, unsigned char *edge_img, int width, int height, size_t img_size)
{
    unsigned char *p;
    unsigned char *pg;
    int16_t x = 0, y = 0;
    int16_t sum1, sum2;
    int8_t kernel_i, kernel_j;
    int8_t edge_matrix1[3][3] = {{-1, 0, 1},
                                {-2, 0, 2},
                                {-1, 0, 1}};
    int8_t edge_matrix2[3][3] = {{1, 2, 1},
                                {0, 0, 0},
                                {-1, -2, -1}};

    for(p = img, pg = edge_img; p != img + img_size; p ++, pg++)
    {
        sum1 = 0;
        sum2 = 0;

        if (!(x == 0 || y == 0 || x == width - 1 || y == height -1))
        {
            for (kernel_i = -1; kernel_i <= 1; kernel_i += 1)
            {
                for (kernel_j = -1; kernel_j <= 1; kernel_j++)
                {
                    sum1 += *(p + kernel_j + kernel_i*width) * edge_matrix1[kernel_i+1][kernel_j+1];
                    sum2 += *(p + kernel_j + kernel_i*width) * edge_matrix2[kernel_i+1][kernel_j+1];
                }
            }

            *pg = (uint8_t) sqrt(pow(sum1, 2.0) + pow(sum2, 2.0));
        }
        else
        {
            *pg = (uint8_t) 0;
        }
        
        x++;
        if(x == width)
        {
            x = 0;
            y++;
        }
    }
}

void acc_init(unsigned char * acc, int rho)
{
    int16_t i, j;

    for (i = 0; i < THETA2; i++)
    {
        for (j = 0; j < rho; j++)
        {
            *(acc + i * rho + j) = 0;
        }
    }
}

int16_t max_acc(unsigned char *acc0, unsigned char *acc1, int rho)
{
    int16_t i, j;
    int16_t acc_max = 0;
    
    for (i = 0; i < THETA2; i++)
    {
        for (j = 0; j < rho; j++)
        {
            if(*(acc0 + i * rho + j) > acc_max)
            {
            	acc_max = *(acc0 + i * rho + j);
            }
            
            if(*(acc1 + i * rho + j) > acc_max)
            {
            	acc_max = *(acc1 + i * rho + j);
            }
        }
    }

    return acc_max;
}

void draw_lines(unsigned char *img, unsigned char *acc0, unsigned char *acc1, int rho, int width, int height, int16_t acc_max, float draw_factor)
{
    int16_t acc_tres;
    int16_t i, j, j_tmp;
    double a, b;
    double x0, x1, y0, y1;
    double slope;
    double intercept;
    int16_t x, y;
    int16_t y_new, y_old = 0;

    acc_tres = draw_factor * acc_max;

    for (i = 0; i < rho; i++)
    {
        for (j= 0; j < THETA2; j++)
        {
            j_tmp = 2 * j;
            
            if (*(acc0 + j * rho + i) >= acc_tres) 
            {
            	//printf("theta = " << theta << ", rho = " << rho);
               if (j_tmp >= THETA)
                {
                    a = cos((j_tmp-THETA)*M_PI/180);
                    b = sin((j_tmp-THETA)*M_PI/180);
                    x0 = - b * i;
                    y0 = a * i;
                }
                else
                {
                    a = cos(j_tmp*M_PI/180);
                    b = sin(j_tmp*M_PI/180);
                    x0 = b * i;
                    y0 = - a * i;
                }


                x1 = x0 + 10*a;
                y1 = y0 + 10*b;
                
                if (a == 0) 
                    slope = height;
                else 
                    slope = b/a;
                    
                intercept = y1 - slope * x1;

                //printf("x0 = %lf, y0 = %lf, theta = %d, rho = %d, slope = %lf, intercept = %lf\n", x0, y0, theta, rho, slope, intercept);

                if (slope >= height || slope <= -height)
                {
                    for (y = 0; y < height; y++)
                    {
                        *(img + 3*(y*width + i)) = 255;
                        *(img + 3*(y*width + i) + 1) = 0;
                        *(img + 3*(y*width + i) + 2) = 0; 
                    }
                }
                else
                {
                    for (x = 0; x < width; x++) 
                    {
                        y = (int)(slope * x + intercept);
                        y_new = y;
                        
                        // printf("x = %d,  y = %d \n", x, y);
                        if (y >= 0 && y < height) 
                        {
                            if (y_new > y_old + 1 && y_old >= 0 && x != 0 && slope > 0) 
                            {
                                for(y = y_old + 1; y < y_new; y++)
                                {
                                    *(img + 3*(y*width + x)) = 255;
                                    *(img + 3*(y*width + x) + 1) = 0;
                                    *(img + 3*(y*width + x) + 2) = 0;
                                }
                            }
                            else if (y_new < y_old - 1 && y_old < height && x != 0 && slope < 0) 
                            {
                                for(y = y_old - 1; y > y_new; y--)
                                {
                                    *(img + 3*(y*width + x)) = 255;
                                    *(img + 3*(y*width + x) + 1) = 0;
                                    *(img + 3*(y*width + x) + 2) = 0;
                                }
                            }
                            
                            *(img + 3*(y*width + x)) = 255;
                            *(img + 3*(y*width + x) + 1) = 0;
                            *(img + 3*(y*width + x) + 2) = 0;
                        }
                        else if (y_old > 0 && y_old < height && slope > 0)
                        {
                            for(y = y_old; y < height; y++)
                            {
                                *(img + 3*(y*width + x)) = 255;
                                *(img + 3*(y*width + x) + 1) = 0;
                                *(img + 3*(y*width + x) + 2) = 0;
                            }
                        }
                        else if (y_old > 0 && y_old < height && slope < 0)
                        {
                            for(y = y_old; y >= 0; y--)
                            {
                                *(img + 3*(y*width + x)) = 255;
                                *(img + 3*(y*width + x) + 1) = 0;
                                *(img + 3*(y*width + x) + 2) = 0;
                            }
                        }

                        y_old = y_new;
                    }
                }
                
                y_new = 0;
                y_old = 0;
            }
            
            j_tmp++;
            
            if (*(acc1 + j * rho + i) >= acc_tres) 
            {
            	//printf("theta = " << theta << ", rho = " << rho);
               if (j_tmp >= THETA)
                {
                    a = cos((j_tmp-THETA)*M_PI/180);
                    b = sin((j_tmp-THETA)*M_PI/180);
                    x0 = - b * i;
                    y0 = a * i;
                }
                else
                {
                    a = cos(j_tmp*M_PI/180);
                    b = sin(j_tmp*M_PI/180);
                    x0 = b * i;
                    y0 = - a * i;
                }


                x1 = x0 + 10*a;
                y1 = y0 + 10*b;
                
                if (a == 0) 
                    slope = height;
                else 
                    slope = b/a;
                    
                intercept = y1 - slope * x1;

                //printf("x0 = %lf, y0 = %lf, theta = %d, rho = %d, slope = %lf, intercept = %lf\n", x0, y0, theta, rho, slope, intercept);

                if (slope >= height || slope <= -height)
                {
                    for (y = 0; y < height; y++)
                    {
                        *(img + 3*(y*width + i)) = 255;
                        *(img + 3*(y*width + i) + 1) = 0;
                        *(img + 3*(y*width + i) + 2) = 0; 
                    }
                }
                else
                {
                    for (x = 0; x < width; x++) 
                    {
                        y = (int)(slope * x + intercept);
                        y_new = y;
                        
                        // printf("x = %d,  y = %d \n", x, y);
                        if (y >= 0 && y < height) 
                        {
                            if (y_new > y_old + 1 && y_old >= 0 && x != 0 && slope > 0) 
                            {
                                for(y = y_old + 1; y < y_new; y++)
                                {
                                    *(img + 3*(y*width + x)) = 255;
                                    *(img + 3*(y*width + x) + 1) = 0;
                                    *(img + 3*(y*width + x) + 2) = 0;
                                }
                            }
                            else if (y_new < y_old - 1 && y_old < height && x != 0 && slope < 0) 
                            {
                                for(y = y_old - 1; y > y_new; y--)
                                {
                                    *(img + 3*(y*width + x)) = 255;
                                    *(img + 3*(y*width + x) + 1) = 0;
                                    *(img + 3*(y*width + x) + 2) = 0;
                                }
                            }
                            
                            *(img + 3*(y*width + x)) = 255;
                            *(img + 3*(y*width + x) + 1) = 0;
                            *(img + 3*(y*width + x) + 2) = 0;
                        }
                        else if (y_old > 0 && y_old < height && slope > 0)
                        {
                            for(y = y_old; y < height; y++)
                            {
                                *(img + 3*(y*width + x)) = 255;
                                *(img + 3*(y*width + x) + 1) = 0;
                                *(img + 3*(y*width + x) + 2) = 0;
                            }
                        }
                        else if (y_old > 0 && y_old < height && slope < 0)
                        {
                            for(y = y_old; y >= 0; y--)
                            {
                                *(img + 3*(y*width + x)) = 255;
                                *(img + 3*(y*width + x) + 1) = 0;
                                *(img + 3*(y*width + x) + 2) = 0;
                            }
                        }

                        y_old = y_new;
                    }
                }
                
                y_new = 0;
                y_old = 0;
            }
        }
    }
}
