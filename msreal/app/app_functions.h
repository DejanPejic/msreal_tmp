#ifndef _HOUGH_FUNCTIONS_HPP_
#define _HOUGH_FUNCTIONS_HPP_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <math.h>

#define ACC0_BRAM 0
#define ACC1_BRAM 1
#define IMG_BRAM 2

#define RHO 442
#define THETA 180
#define THETA2 135
#define THRESHOLD 149

#define START_REG 0
#define RESET_REG 4
#define WIDTH_REG 8
#define HEIGHT_REG 12
#define RHO_REG 16
#define THRESHOLD_REG 20
#define READY_REG 24
	
void copying(unsigned char *img, unsigned char *same_img, size_t img_size);
void monochromating(unsigned char *img, unsigned char *gray_img, size_t img_size);
void edging(unsigned char *img, unsigned char *edge_img, int width, int height, size_t img_size);

void acc_init(unsigned char *acc, int rho);
int16_t max_acc(unsigned char *acc0, unsigned char *acc1, int rho);
void draw_lines(unsigned char *img, unsigned char *acc0, unsigned char *acc1, int rho, int width, int height, int16_t acc_max, float draw_factor);

void write_bram(int bram_n, unsigned char * val, int length);
void read_bram(int bram_n, unsigned char *val, int length);
void write_hard(unsigned char addr, int val);
int read_hard(unsigned char addr);

#endif