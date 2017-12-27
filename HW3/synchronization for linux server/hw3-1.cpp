// Student ID: 0416208
// Name      : 黃士軒
// Date      : 2017.11.03

#include "bmpReader.h"
#include "bmpReader.cpp"
#include <stdio.h>
#include <iostream>
#include <math.h>
#include <pthread.h>
#include <semaphore.h>
using namespace std;

#define MYRED	2
#define MYGREEN 1
#define MYBLUE	0

int imgWidth, imgHeight;
int FILTER_SIZE;
int FILTER_SCALE;
int *filter_G;

const char *inputfile_name[5] = {
	"input1.bmp",
	"input2.bmp",
	"input3.bmp",
	"input4.bmp",
	"input5.bmp"
};
const char *outputBlur_name[5] = {
	"Blur1.bmp",
	"Blur2.bmp",
	"Blur3.bmp",
	"Blur4.bmp",
	"Blur5.bmp"
};


unsigned char *pic_in, *pic_grey, *pic_blur, *pic_final;
//int *zero_matrix;
/*
unsigned char RGB2grey(int w, int h)
{
    //RGB 3個為一個描述格
	int tmp = (
		pic_in[3 * (h*imgWidth + w) + MYRED] +
		pic_in[3 * (h*imgWidth + w) + MYGREEN] +
		pic_in[3 * (h*imgWidth + w) + MYBLUE] )/3;

	if (tmp < 0) tmp = 0;
	if (tmp > 255) tmp = 255;
	return (unsigned char)tmp;
}
*/

/*
unsigned char GaussianFilter(int w, int h)
{
    int tmp = 0;
    int a, b;
    int ws = (int)sqrt((float)FILTER_SIZE);    //ws * ws matrix
    for (int j = 0; j<ws; j++)      //
        for (int i = 0; i<ws; i++)
        {
            a = w + i - (ws / 2);
            b = h + j - (ws / 2);
            
            // detect for borders of the image
            if (a<0 || b<0 || a>imgWidth || b>imgHeight) continue;
            
            tmp += filter_G[j*ws + i] * pic_grey[b*imgWidth + a];
        };
    tmp /= FILTER_SCALE;
    if (tmp < 0) tmp = 0;
    if (tmp > 255) tmp = 255;
    return (unsigned char)tmp;
}
*/


//2 mutex lock to rgb convert
//pthread_mutex_t mutex_one_write;
//pthread_mutex_t mutex_second_write;
//pthread_cond_t cond_one;
//pthread_cond_t cond_second;

sem_t blur_sem, blur_sem_half;
bool *imghight_gray_done;

void *RGB2grey(void *argv){
    
    //only run one line per time, start from the secone column
    for (int j = 0; j < imgHeight; j=j+2) {
        for (int i = 0; i < imgWidth; i++){
            
            int tmp = (
                       pic_in[3 * (j*imgWidth + i) + MYRED] +
                       pic_in[3 * (j*imgWidth + i) + MYGREEN] +
                       pic_in[3 * (j*imgWidth + i) + MYBLUE] )/3;
            
            if (tmp < 0) tmp = 0;
            if (tmp > 255) tmp = 255;
            
            pic_grey[j*imgWidth + i] = (unsigned char)tmp;
        }
        
        imghight_gray_done[j] = true;
    }
    
    return NULL;
}


void *RGB2grey_half(void *argv){
    //only do the first column convert
    
    //only run the first line
    for (int j = 1; j < imgHeight; j=j+2) {
        for (int i = 0; i < imgWidth; i++){
            
            int tmp = (
                       pic_in[3 * (j*imgWidth + i) + MYRED] +
                       pic_in[3 * (j*imgWidth + i) + MYGREEN] +
                       pic_in[3 * (j*imgWidth + i) + MYBLUE] )/3;
            
            if (tmp < 0) tmp = 0;
            if (tmp > 255) tmp = 255;
            
            pic_grey[j*imgWidth + i] = (unsigned char)tmp;
            
        }
        imghight_gray_done[j] = true;
    }
    
    return NULL;
}


void *GaussianFilter(void *argv){

    int ws = (int)sqrt((float)FILTER_SIZE);    //ws * ws matrix
    int border = (int) (ws-1)/2;
    // do the filter column by column
    for (int h = 0; h < imgHeight; h=h+2) {
        
        if(h < imgHeight - border)
            while( imghight_gray_done[h+border] == false || imghight_gray_done[h+border-1] == false);

        for (int w = 0; w < imgWidth; w++){
            
            int tmp = 0;
            int a, b;
            
            for (int j = 0; j< ws; j++)
                for (int i = 0; i<ws; i++)
                {
                    a = w + i - (ws / 2);
                    b = h + j - (ws / 2);
                    
                    // detect for borders of the image
                    if (a<0 || b<0 || a>=imgWidth || b>=imgHeight) continue;
                    
                    tmp += filter_G[j*ws + i] * pic_grey[b*imgWidth + a];
                };
            tmp /= FILTER_SCALE;
            if (tmp < 0) tmp = 0;
            if (tmp > 255) tmp = 255;

            pic_blur[h*imgWidth + w] = (unsigned char)tmp;

        }
        
    }
    sem_post(&blur_sem);
    return NULL;
}

void *GaussianFilter_half(void *argv){
    
    int ws = (int)sqrt((float)FILTER_SIZE);    //ws * ws matrix
    int border = (int) (ws-1)/2;
    
    // do the filter column by column
    for (int h = 1; h < imgHeight; h=h+2) {
        
        if(h < imgHeight - border)
            while( imghight_gray_done[h+border] == false || imghight_gray_done[h+border-1] == false);
        
        for (int w = 0; w < imgWidth; w++){
            
            int tmp = 0;
            int a, b;
            for (int j = 0; j< ws; j++)      //
                for (int i = 0; i<ws; i++)
                {
                    a = w + i - (ws / 2);
                    b = h + j - (ws / 2);
                    
                    // detect for borders of the image
                    if (a<0 || b<0 || a>=imgWidth || b>=imgHeight) continue;
                    
                    tmp += filter_G[j*ws + i] * pic_grey[b*imgWidth + a];
                };
            tmp /= FILTER_SCALE;
            if (tmp < 0) tmp = 0;
            if (tmp > 255) tmp = 255;
            
            pic_blur[h*imgWidth + w] = (unsigned char)tmp;
            
        }
        
    }
    sem_post(&blur_sem_half);
    return NULL;
}


void *extend_size(void *argv){
    
    //int job_load = * (int *) argv;
    
    sem_wait(&blur_sem);
    for (int j = 0; j < imgHeight; j=j+2) {
        
        for (int i = 0; i<imgWidth; i++){
            pic_final[3 * (j*imgWidth + i) + MYRED] = pic_blur[j*imgWidth + i];
            pic_final[3 * (j*imgWidth + i) + MYGREEN] = pic_blur[j*imgWidth + i];
            pic_final[3 * (j*imgWidth + i) + MYBLUE] = pic_blur[j*imgWidth + i];
        }
    }
    
    return NULL;
}

void *extend_size_half(void *argv){
    
    //int job_load = * (int *) argv;
    
    sem_wait(&blur_sem_half);
    for (int j = 1; j < imgHeight; j=j+2) {
        
        for (int i = 0; i<imgWidth; i++){
            pic_final[3 * (j*imgWidth + i) + MYRED] = pic_blur[j*imgWidth + i];
            pic_final[3 * (j*imgWidth + i) + MYGREEN] = pic_blur[j*imgWidth + i];
            pic_final[3 * (j*imgWidth + i) + MYBLUE] = pic_blur[j*imgWidth + i];
        }
    }
    
    return NULL;
}


int main()
{
	// read mask file
	FILE* mask;
	mask = fopen("mask_Gaussian.txt", "r");
	fscanf(mask, "%d", &FILTER_SIZE);
	fscanf(mask, "%d", &FILTER_SCALE);

	filter_G = new int[FILTER_SIZE];
	for (int i = 0; i<FILTER_SIZE; i++)
		fscanf(mask, "%d", &filter_G[i]);
	fclose(mask);
    
    /*
    pthread_t *thread_rgb2grey1 = NULL;
    pthread_t *thread_rgb2grey2 = NULL;
    pthread_t *thread_gaussian = NULL;
     */
    pthread_t thread_rgb2grey1, thread_rgb2grey2, thread_gaussian1, thread_gaussian2 , thread_extend_size_1, thread_extend_size_2;

    
	BmpReader* bmpReader = new BmpReader();
	for (int k = 0; k<5; k++){
        //thread setting
        //thread_rgb2grey2 = (pthread_t*) malloc(sizeof(pthread_t));
        //thread_gaussian = (pthread_t*) malloc(sizeof(pthread_t));
        //pthread_t thread_rgb2grey1, thread_extend_size_1, thread_extend_size_2;
        
        //setting the parallels attritube
        //pthread_attr_t attr;
        //pthread_attr_init(&attr);
        //pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
		// read input BMP file
		pic_in = bmpReader->ReadBMP(inputfile_name[k], &imgWidth, &imgHeight);
		// allocate space for output image
		pic_grey = (unsigned char*)malloc(imgWidth*imgHeight*sizeof(unsigned char));
		pic_blur = (unsigned char*)malloc(imgWidth*imgHeight*sizeof(unsigned char));
		pic_final = (unsigned char*)malloc(3 * imgWidth*imgHeight*sizeof(unsigned char));
		imghight_gray_done = (bool*)malloc(imgHeight*sizeof(bool));
        
        //pthread_mutex_init(&mutex_first, NULL);
        //pthread_mutex_init(&mutex_one_write, NULL);
        //pthread_mutex_init(&mutex_second_write, NULL);
        //pthread_cond_init(&cond_one, NULL);
        //pthread_cond_init(&cond_second, NULL);
        
        sem_init(&blur_sem, 0, 0);
        sem_init(&blur_sem_half, 0, 0);
		//convert RGB image to grey image
        //seperate to 2 thread first time burst do the two column convert
        //then do it one by one
        for(int i = 0; i < imgHeight; i++) {
            imghight_gray_done[i] = false;
        }
        
        pthread_create(&thread_rgb2grey1, NULL, &RGB2grey, NULL);       //do the convert specific on first column then done
        pthread_create(&thread_rgb2grey2, NULL, &RGB2grey_half, NULL);

		//apply the Gaussian filter to the image with RGB respectively
        pthread_create(&thread_gaussian1, NULL, &GaussianFilter, NULL);
        pthread_create(&thread_gaussian2, NULL, &GaussianFilter_half, NULL);
        

        pthread_join(thread_rgb2grey1, NULL);
        pthread_join(thread_rgb2grey2, NULL);
  
        //pthread_join(thread_gaussian1, NULL);
        //pthread_join(thread_gaussian2, NULL);
        //extend the size form WxHx1 to WxHx3
        //use pthread give the job half and half
        pthread_create(&thread_extend_size_1, NULL, &extend_size, NULL);
        pthread_create(&thread_extend_size_2, NULL, &extend_size_half, NULL);
        
        
        
        pthread_join(thread_extend_size_1, NULL);
        pthread_join(thread_extend_size_2, NULL);

        free(pic_in);
        free(pic_grey);
        free(imghight_gray_done);

		// write output BMP file
		bmpReader->WriteBMP(outputBlur_name[k], imgWidth, imgHeight, pic_final);

		//free memory space
        
        sem_close(&blur_sem);
        sem_close(&blur_sem_half);
		free(pic_blur);
		free(pic_final);

	}

    
	return 0;
}
