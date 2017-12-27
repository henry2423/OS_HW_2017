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
int *filter_Gx, *filter_Gy;

const char *inputfile_name[5] = {
	"input1.bmp",
	"input2.bmp",
	"input3.bmp",
	"input4.bmp",
	"input5.bmp"
};

const char *outputSobel_name[5] = {
	"Sobel1.bmp",
	"Sobel2.bmp",
	"Sobel3.bmp",
	"Sobel4.bmp",
	"Sobel5.bmp"
};

unsigned char *pic_in, *pic_grey ,*pic_final;
int *pic_x, *pic_y;
bool *imghight_comp_done;
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

pthread_mutex_t mutex_sobel, mutex_sobel_half;
pthread_cond_t cond_sobel, cond_sobel_half;
//sem_t sem_compu, sem_compu_half;
//int compu_done, compu_done_half;

void *RGB2grey(void *argv){
    
    //only run one line per time, start from the secone column
    for (int j = 0; j < imgHeight; j=j+2) {
        for (int i = 0; i < imgWidth; i++){
            
            int tmp = (
                       pic_in[3 * (j*imgWidth + i) + MYRED] +
                       pic_in[3 * (j*imgWidth + i) + MYGREEN] +
                       pic_in[3 * (j*imgWidth + i) + MYBLUE] )/3;
            
            if (tmp < 0) tmp = 0;
            else if (tmp > 255) tmp = 255;
            
            pic_grey[j*imgWidth + i] = (unsigned char)tmp;
            //pic_sobel[j*imgWidth + i] = 0;
            //imghight_comp_done[j*imgWidth + i] = 0;
        }
        
        imghight_comp_done[j] = false;
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
            else if (tmp > 255) tmp = 255;
            
            pic_grey[j*imgWidth + i] = (unsigned char)tmp;
            //pic_sobel[j*imgWidth + i] = 0;
            //imghight_comp_done[j*imgWidth + i] = 0;
        }
        imghight_comp_done[j] = false;
    }
    
    return NULL;
}


void *EdgeFilter_Gx(void *argv){


    // do the filter column by column
    for (int h = 0; h < (int ) imgHeight; h=h+2) {

        for (int w = 0; w < imgWidth; w++){
            
            int tmp = 0;
            int a, b;
            int ws = (int)sqrt((float)FILTER_SIZE);    //ws * ws matrix
            for (int j = 0; j< ws; j++)      //
                for (int i = 0; i<ws; i++)
                {
                    a = w + i - (ws / 2);
                    b = h + j - (ws / 2);
                    
                    // detect for borders of the image
                    if (a<0 || b<0 || a>=imgWidth || b>=imgHeight) continue;
                    
                    tmp += filter_Gx[j*ws + i] * pic_grey[b*imgWidth + a];
                };
            
            if (tmp < 0) tmp = 0;
            else if (tmp > 255) tmp = 255;
            //tmp = pow(tmp, 2);
            pic_x[h*imgWidth + w] = tmp;
            //sem_post(&sem_compu);
        }
    }
    
    return NULL;
}

void *EdgeFilter_Gy(void *argv){
    
    
    // do the filter column by column
    for (int h = 0; h < (int ) imgHeight; h=h+2) {
        
        for (int w = 0; w < imgWidth; w++){
            
            int tmp = 0;
            int a, b;
            int ws = (int)sqrt((float)FILTER_SIZE);    //ws * ws matrix
            for (int j = 0; j< ws; j++)      //
                for (int i = 0; i<ws; i++)
                {
                    a = w + i - (ws / 2);
                    b = h + j - (ws / 2);
                    
                    // detect for borders of the image
                    if (a<0 || b<0 || a>=imgWidth || b>=imgHeight) continue;
                    
                    tmp += filter_Gy[j*ws + i] * pic_grey[b*imgWidth + a];
                };
            
            if (tmp < 0) tmp = 0;
            else if (tmp > 255) tmp = 255;
            //tmp = pow(tmp, 2);
            pic_y[h*imgWidth + w] = tmp;
        }
    }

    return NULL;
}

void *EdgeFilter_half_Gx(void *argv){

    // do the filter column by column
    for (int h = 1; h < imgHeight; h=h+2) {

        for (int w = 0; w < imgWidth; w++){
            
            int tmp = 0;
            int a, b;
            int ws = (int)sqrt((float)FILTER_SIZE);    //ws * ws matrix
            for (int j = 0; j< ws; j++)      //
                for (int i = 0; i<ws; i++)
                {
                    a = w + i - (ws / 2);
                    b = h + j - (ws / 2);
                    
                    // detect for borders of the image
                    if (a<0 || b<0 || a>=imgWidth || b>=imgHeight) continue;
                    
                    tmp += filter_Gx[j*ws + i] * pic_grey[b*imgWidth + a];
                };
            
            if (tmp < 0) tmp = 0;
            else if (tmp > 255) tmp = 255;
            //tmp = pow(tmp, 2);
            pic_x[h*imgWidth + w] = tmp;
            
        }
    }
    
    return NULL;
}

void *EdgeFilter_half_Gy(void *argv){
    
    // do the filter column by column
    for (int h = 1; h < imgHeight; h=h+2) {
        
        for (int w = 0; w < imgWidth; w++){
            
            int tmp = 0;
            int a, b;
            int ws = (int)sqrt((float)FILTER_SIZE);    //ws * ws matrix
            for (int j = 0; j< ws; j++)      //
                for (int i = 0; i<ws; i++)
                {
                    a = w + i - (ws / 2);
                    b = h + j - (ws / 2);
                    
                    // detect for borders of the image
                    if (a<0 || b<0 || a>=imgWidth || b>=imgHeight) continue;
                    
                    tmp += filter_Gy[j*ws + i] * pic_grey[b*imgWidth + a];
                };
           
            if (tmp < 0) tmp = 0;
            else if (tmp > 255) tmp = 255;
            //tmp = pow(tmp, 2);
            pic_y[h*imgWidth + w] = tmp;
            
        }
    }

    return NULL;
}

void *thread_compute(void *argv){
    
    
    
    for (int h = 0; h < imgHeight; h=h+2) {
        for (int w = 0; w < imgWidth; w++){
            pic_x[h*imgWidth + w] = pow(pic_x[h*imgWidth + w], 2);
            pic_y[h*imgWidth + w] = pow(pic_y[h*imgWidth + w], 2);
            //pic_x[h*imgWidth + w] = sqrt(pic_x[h*imgWidth + w] + pic_y[h*imgWidth + w]);
        }
        pthread_mutex_lock(&mutex_sobel);
        imghight_comp_done[h] = true;
        pthread_cond_signal(&cond_sobel);
        pthread_mutex_unlock(&mutex_sobel);
        
    }
    
    return NULL;
}

void *thread_compute_half(void *argv){
    
    
    
    for (int h = 1; h < imgHeight; h=h+2) {
        for (int w = 0; w < imgWidth; w++){
            pic_x[h*imgWidth + w] = pow(pic_x[h*imgWidth + w], 2);
            pic_y[h*imgWidth + w] = pow(pic_y[h*imgWidth + w], 2);
            //pic_x[h*imgWidth + w] = sqrt(pic_x[h*imgWidth + w] + pic_y[h*imgWidth + w]);
        }
        
        pthread_mutex_lock(&mutex_sobel_half);
        imghight_comp_done[h] = true;
        pthread_cond_signal(&cond_sobel_half);
        pthread_mutex_unlock(&mutex_sobel_half);
        
    }
    
    return NULL;
}

void *extend_size(void *argv){
    
    /*
    for(int i = 0; i < imgHeight*imgWidth; i = i + 2) {
    
        sem_wait(&sem_compu);
        pic_x[i] = sqrt(pic_x[i] + pic_y[i]);

        if (pic_x[i] < 0) pic_x[i]  = 0;
        else if (pic_x[i]  > 255) pic_x[i]  = 255;
        pic_final[3 * i + MYRED] = pic_x[i];
        pic_final[3 * i + MYGREEN] = pic_x[i];
        pic_final[3 * i + MYBLUE] = pic_x[i];
    }
    */
    
    for (int j = 0; j < imgHeight; j=j+2) {
        
        pthread_mutex_lock(&mutex_sobel);
        while (imghight_comp_done[j] == false)
            pthread_cond_wait(&cond_sobel,&mutex_sobel);
        pthread_mutex_unlock(&mutex_sobel);
        
        
        for (int i = 0; i<imgWidth; i++){
            
            //sem_wait(&sem_compu);
            pic_x[j*imgWidth + i] = sqrt(pic_x[j*imgWidth + i] + pic_y[j*imgWidth + i]);
            if (pic_x[j*imgWidth + i] < 0) pic_x[j*imgWidth + i] = 0;
            else if (pic_x[j*imgWidth + i] > 255) pic_x[j*imgWidth + i] = 255;
            pic_final[3 * (j*imgWidth + i) + MYRED] = pic_x[j*imgWidth + i];
            pic_final[3 * (j*imgWidth + i) + MYGREEN] = pic_x[j*imgWidth + i];
            pic_final[3 * (j*imgWidth + i) + MYBLUE] = pic_x[j*imgWidth + i];
        }
    }
    
    return NULL;
}

void *extend_size_half(void *argv){
    
    /*
    for(int i = 1; i < imgHeight*imgWidth; i = i + 2) {
        
        sem_wait(&sem_compu_half);
        pic_x[i] = sqrt(pic_x[i] + pic_y[i]);
        if (pic_x[i] < 0) pic_x[i]  = 0;
        else if (pic_x[i]  > 255) pic_x[i]  = 255;
        pic_final[3 * i + MYRED] = pic_x[i];
        pic_final[3 * i + MYGREEN] = pic_x[i];
        pic_final[3 * i + MYBLUE] = pic_x[i];
    }
    */

    for (int j = 1; j < imgHeight; j=j+2) {
        pthread_mutex_lock(&mutex_sobel_half);
        while (imghight_comp_done[j] == false)
            pthread_cond_wait(&cond_sobel_half,&mutex_sobel_half);
        pthread_mutex_unlock(&mutex_sobel_half);
        
        
        for (int i = 0; i<imgWidth; i++){
            
            //sem_wait(&sem_compu_half);
            pic_x[j*imgWidth + i] = sqrt(pic_x[j*imgWidth + i] + pic_y[j*imgWidth + i]);
            if (pic_x[j*imgWidth + i] < 0) pic_x[j*imgWidth + i] = 0;
            else if (pic_x[j*imgWidth + i] > 255) pic_x[j*imgWidth + i] = 255;
            pic_final[3 * (j*imgWidth + i) + MYRED] = pic_x[j*imgWidth + i];
            pic_final[3 * (j*imgWidth + i) + MYGREEN] = pic_x[j*imgWidth + i];
            pic_final[3 * (j*imgWidth + i) + MYBLUE] = pic_x[j*imgWidth + i];
        }
    }
    
    return NULL;
}


int main()
{
	// read mask file
	FILE* mask;
	mask = fopen("mask_Sobel.txt", "r");
	fscanf(mask, "%d", &FILTER_SIZE);

	filter_Gx = new int[FILTER_SIZE];
    filter_Gy = new int[FILTER_SIZE];
	for (int i = 0; i<FILTER_SIZE; i++)
		fscanf(mask, "%d", &filter_Gx[i]);
    for (int i = 0; i<FILTER_SIZE; i++)
        fscanf(mask, "%d", &filter_Gy[i]);
	fclose(mask);
    
    /*
    pthread_t *thread_rgb2grey1 = NULL;
    pthread_t *thread_rgb2grey2 = NULL;
    pthread_t *thread_gaussian = NULL;
     */
    pthread_t thread_rgb2grey1, thread_rgb2grey2, thread_gaussian1, thread_gaussian2, thread_gaussian3, thread_gaussian4, thread_compu1, thread_compu2, thread_extend_size_1, thread_extend_size_2;
    
    
	BmpReader* bmpReader = new BmpReader();
	for (int k = 0; k<5; k++){

		// read input BMP file
		pic_in = bmpReader->ReadBMP(inputfile_name[k], &imgWidth, &imgHeight);
		// allocate space for output image
		pic_grey = (unsigned char*)malloc(imgWidth*imgHeight*sizeof(unsigned char));
        pic_x = (int*)malloc(imgWidth*imgHeight*sizeof(int));
        pic_y = (int*)malloc(imgWidth*imgHeight*sizeof(int));
        //pic_sobel = (int*)malloc(imgWidth*imgHeight*sizeof(int));
		imghight_comp_done = (bool*)malloc(imgHeight*sizeof(bool));
        
        
        //sem_init(&sem_compu, 0, 0);
        //sem_init(&sem_compu_half, 0, 0);
        //sem_compu = *sem_open("sem", O_CREAT,0644,1);
        //sem_compu_half = *sem_open("sem_half", O_CREAT,0644,1);
		//convert RGB image to grey image
        //seperate to 2 thread first time burst do the two column convert
        //then do it one by one
        /*
        for(int i = 0; i < imgHeight; i++) {
            imghight_gray_done[i] = false;
        }
        */
        pthread_create(&thread_rgb2grey1, NULL, &RGB2grey, NULL);       //do the convert specific on first column then done
        pthread_create(&thread_rgb2grey2, NULL, &RGB2grey_half, NULL);
        
        
        
        pthread_join(thread_rgb2grey1, NULL);
        pthread_join(thread_rgb2grey2, NULL);
        
		//apply the Gaussian filter to the image with RGB respectively
        pthread_create(&thread_gaussian1, NULL, &EdgeFilter_Gx, NULL);
        pthread_create(&thread_gaussian2, NULL, &EdgeFilter_half_Gx, NULL);
        pthread_create(&thread_gaussian3, NULL, &EdgeFilter_Gy, NULL);
        pthread_create(&thread_gaussian4, NULL, &EdgeFilter_half_Gy, NULL);
        
        /*
        for(int i = 0; i < imgWidth*imgHeight; i++) {
            pic_sobel[i] = 0;
        }
        */
        //compu_done = 0;
        //compu_done_half = 0;
        pthread_mutex_init(&mutex_sobel, NULL);
        pthread_mutex_init(&mutex_sobel_half, NULL);
        pthread_cond_init(&cond_sobel, NULL);
        pthread_cond_init(&cond_sobel_half, NULL);
        pic_final = (unsigned char*)malloc(3 * imgWidth*imgHeight*sizeof(unsigned char));
        
        
        pthread_join(thread_gaussian1, NULL);
        pthread_join(thread_gaussian3, NULL);
        pthread_create(&thread_compu1, NULL, &thread_compute, NULL);
        pthread_create(&thread_extend_size_1, NULL, &extend_size, NULL);

        
        pthread_join(thread_gaussian2, NULL);
        pthread_join(thread_gaussian4, NULL);
        pthread_create(&thread_compu2, NULL, &thread_compute_half, NULL);
        pthread_create(&thread_extend_size_2, NULL, &extend_size_half, NULL);
        
        free(pic_in);
        free(pic_grey);
        pthread_join(thread_extend_size_1, NULL);
        pthread_join(thread_extend_size_2, NULL);
        


        
		// write output BMP file
		bmpReader->WriteBMP(outputSobel_name[k], imgWidth, imgHeight, pic_final);

		//free memory space
        //sem_destroy(&sem_compu);
        //sem_destroy(&sem_compu_half);
        pthread_mutex_destroy(&mutex_sobel);
        pthread_mutex_destroy(&mutex_sobel_half);
        pthread_cond_destroy(&cond_sobel);
        pthread_cond_destroy(&cond_sobel_half);
        free(imghight_comp_done);
		//free(pic_sobel);
        free(pic_x);
        free(pic_y);
		free(pic_final);

	}

    
	return 0;
}
