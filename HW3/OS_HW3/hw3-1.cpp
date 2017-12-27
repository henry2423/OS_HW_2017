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
	"/Users/Henry/Downloads/OS_HW3/input1.bmp",
	"/Users/Henry/Downloads/OS_HW3/input2.bmp",
	"/Users/Henry/Downloads/OS_HW3/input3.bmp",
	"/Users/Henry/Downloads/OS_HW3/input4.bmp",
	"/Users/Henry/Downloads/OS_HW3/input5.bmp"
};
const char *outputBlur_name[5] = {
	"/Users/Henry/Downloads/OS_HW3/Blur1.bmp",
	"/Users/Henry/Downloads/OS_HW3/Blur2.bmp",
	"/Users/Henry/Downloads/OS_HW3/Blur3.bmp",
	"/Users/Henry/Downloads/OS_HW3/Blur4.bmp",
	"/Users/Henry/Downloads/OS_HW3/Blur5.bmp"
};

/*
const char *outputSobel_name[5] = {
	"Sobel1.bmp",
	"Sobel2.bmp",
	"Sobel3.bmp",
	"Sobel4.bmp",
	"Sobel5.bmp"
};*/

unsigned char *pic_in, *pic_grey, *pic_blur, *pic_final;
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
pthread_mutex_t mutex_first = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_one_column = PTHREAD_MUTEX_INITIALIZER;


void *RGB2grey_firstc(void *argv){
    //only do the first column convert

    pthread_mutex_lock(&mutex_first);
    
    //only run the first line
    for (int j = 0; j < 1; j++) {
        for (int i = 0; i < imgWidth; i++){
            
            int tmp = (
                       pic_in[3 * (j*imgWidth + i) + MYRED] +
                       pic_in[3 * (j*imgWidth + i) + MYGREEN] +
                       pic_in[3 * (j*imgWidth + i) + MYBLUE] )/3;
            
            if (tmp < 0) tmp = 0;
            if (tmp > 255) tmp = 255;
            
            pic_grey[j*imgWidth + i] = (unsigned char)tmp;
            
        }
    }

    pthread_mutex_unlock(&mutex_first);
    
    pthread_exit(0);
    return NULL;
}


void *RGB2grey(void *argv){
    
    //only run one line per time, start from the secone column
    for (int j = 1; j < imgHeight; j++) {
        
        pthread_mutex_lock(&mutex_one_column);
        
        for (int i = 0; i < imgWidth; i++){
            
            int tmp = (
                       pic_in[3 * (j*imgWidth + i) + MYRED] +
                       pic_in[3 * (j*imgWidth + i) + MYGREEN] +
                       pic_in[3 * (j*imgWidth + i) + MYBLUE] )/3;
            
            if (tmp < 0) tmp = 0;
            if (tmp > 255) tmp = 255;
            
            pic_grey[j*imgWidth + i] = (unsigned char)tmp;
            
        }
        
        pthread_mutex_unlock(&mutex_one_column);
        
    }
    
    pthread_exit(0);
    return NULL;
}


void *GaussianFilter(void *argv){

    // do the filter column by column
    for (int h = 0; h < imgHeight; h++) {
        
        pthread_mutex_lock(&mutex_first);
        pthread_mutex_lock(&mutex_one_column);
 
        for (int w = 0; w < imgWidth; w++){
            
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
  
            pic_blur[h*imgWidth + w] = (unsigned char)tmp;
        }

        pthread_mutex_unlock(&mutex_one_column);
        pthread_mutex_unlock(&mutex_first);
        
    }
    
    pthread_exit(0);
    return NULL;
}

void *extend_size(void *argv){
    
    int job_load = * (int *) argv;
    
    //cout << job_load << endl;
    
    for (int j = job_load; j < job_load + imgHeight/2; j++) {
        for (int i = 0; i<imgWidth; i++){
            pic_final[3 * (j*imgWidth + i) + MYRED] = pic_blur[j*imgWidth + i];
            pic_final[3 * (j*imgWidth + i) + MYGREEN] = pic_blur[j*imgWidth + i];
            pic_final[3 * (j*imgWidth + i) + MYBLUE] = pic_blur[j*imgWidth + i];
        }
    }
    
    pthread_exit(0);
    return NULL;
}

int main()
{
	// read mask file
	FILE* mask;
	mask = fopen("/Users/Henry/Downloads/OS_HW3/mask_Gaussian.txt", "r");
	fscanf(mask, "%d", &FILTER_SIZE);
	fscanf(mask, "%d", &FILTER_SCALE);

	filter_G = new int[FILTER_SIZE];
	for (int i = 0; i<FILTER_SIZE; i++)
		fscanf(mask, "%d", &filter_G[i]);
	fclose(mask);
    
    
    //thread setting
    pthread_t thread_rgb2grey1, thread_rgb2grey2, thread_gaussian, thread_extend_size_1, thread_extend_size_2;
    
    //setting the parallels attritube
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    
	BmpReader* bmpReader = new BmpReader();
	for (int k = 0; k<5; k++){
		// read input BMP file
		pic_in = bmpReader->ReadBMP(inputfile_name[k], &imgWidth, &imgHeight);
		// allocate space for output image
		pic_grey = (unsigned char*)malloc(imgWidth*imgHeight*sizeof(unsigned char));
		pic_blur = (unsigned char*)malloc(imgWidth*imgHeight*sizeof(unsigned char));
		pic_final = (unsigned char*)malloc(3 * imgWidth*imgHeight*sizeof(unsigned char));
		
		//convert RGB image to grey image
        //seperate to 2 thread first time burst do the two column convert
        //then do it one by one
        pthread_create(&thread_rgb2grey1, &attr, &RGB2grey_firstc, NULL);      //do the convert specific on first column then done
        pthread_create(&thread_rgb2grey2, &attr, &RGB2grey, NULL);


		//apply the Gaussian filter to the image
        pthread_create(&thread_gaussian, &attr, &GaussianFilter, NULL);

        
        pthread_join(thread_rgb2grey1, NULL);
        pthread_join(thread_rgb2grey2, NULL);
        pthread_join(thread_gaussian, NULL);
        
        
		//extend the size form WxHx1 to WxHx3
        //use pthread give the job half and half
        int from_zero = 0;
        pthread_create(&thread_extend_size_1, &attr, &extend_size, &from_zero);
        int half_job = imgHeight/2;
        pthread_create(&thread_extend_size_2, &attr, &extend_size, &half_job);

        pthread_join(thread_extend_size_1, NULL);
        pthread_join(thread_extend_size_2, NULL);
        
		// write output BMP file
		bmpReader->WriteBMP(outputBlur_name[k], imgWidth, imgHeight, pic_final);

		//free memory space
		free(pic_in);
		free(pic_grey);
		free(pic_blur);
		free(pic_final);
	}

	return 0;
}
