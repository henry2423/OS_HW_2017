//
//  main.cpp
//  share_memory
//
//  Created by Henry on 10/10/2017.
//  Copyright © 2017 henry. All rights reserved.
//

#include <iostream>
#include <stdio.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <sys/time.h>

using namespace std;

int main(int argc, const char * argv[]) {
    
    struct timeval start, end;
    
    int dem;
    printf("Dimesion: ");
    scanf("%d", &dem);
    
    gettimeofday(&start, 0);
    unsigned int checksum = 0;
    
    for(int i = 0; i < dem * dem; i++)
    {
        int multier = dem * (i / dem);
        int multed = i % dem;
        unsigned int sum = 0;
        
        for(int j = 0; j < dem; j++)    //one line
        {
            //printf("%d %d\n",multier + j, multed + (dem * j));
            sum += (multier + j) * (multed + (dem * j));
        }
        
        //printf("%ld \n", sum);
        checksum += sum;
    }
    
    printf("1-process, checksum = %u\n", checksum);
    
    
    gettimeofday(&end, 0);
    int sec = (end.tv_sec - start.tv_sec);
    int usec = (end.tv_usec - start.tv_usec);
    printf("elapsed %f ms\n", sec*1000+(usec/1000.0));

    
    
    // miltiprocess
    
    gettimeofday(&start, 0);
    
    int shm_gt;
    const int SIZE = 4096;
    unsigned int *data_ptr;
    
    shm_gt = shmget(0, SIZE,  IPC_CREAT | 0666);
    data_ptr = (unsigned int *) shmat(shm_gt, NULL, 0);
    if (data_ptr == (void *)-1) {
        printf("shmat error\n");
        return 1;
    }

    pid_t child_pid = -1;
    int proc = 0;
    for(proc ; proc<4; proc++)
    {
        child_pid = fork();
        
        if(child_pid == 0 || child_pid == -1)
            break;
    }
    
    if(child_pid < 0)
    {
        fprintf(stderr, "Fork Failed");
        return 1;
    }
    else if(child_pid == 0)
    {
        //printf("Fork 2. I'm the child %d, my parent is %d %d.\n", getpid(), getppid(), proc);        //Child 2
        
        checksum = 0;
        int child_dem = 0;
        int i = 0;
        
        if(proc < 3)
        {
            child_dem = ((dem*dem) / 4) * (proc + 1);
            i = child_dem - ((dem*dem) / 4);
        }
        else
        {
            child_dem = dem * dem;
            i = ((dem*dem) / 4) * (proc + 1) - ((dem*dem) / 4);
        }
        
        for(i ; i < child_dem; i++)
        {
            int multier = 0 + dem * (i / dem);
            int multed = i % dem;

            for(int j = 0; j < dem; j++)    //one line
            {
                checksum += (multier + j) * (multed + (dem * j));
            }
            
        }
        //printf("%u \n",checksum);
        data_ptr[proc] = checksum;
        return 0;
    }
    else
    {
        //waitpid(child_pid, NULL, 0);
        wait(NULL);
        wait(NULL);
        wait(NULL);
        wait(NULL);
        checksum = 0;
        for (int i = 0; i < 4; i++)
        {
            checksum += data_ptr[i];
        }
        
        gettimeofday(&end, 0);
        sec = (end.tv_sec - start.tv_sec);
        usec = (end.tv_usec - start.tv_usec);
        
        printf("4-process, checksum = %u\n",checksum);
        printf("elapsed %f ms\n", sec*1000+(usec/1000.0));
        
        shmdt((void *) data_ptr);
    }
    
    
    return 0;
}

