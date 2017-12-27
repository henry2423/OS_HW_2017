//
//  main.cpp
//  fork
//
//  Created by Henry on 04/10/2017.
//  Copyright © 2017 henry. All rights reserved.
//

#include <iostream>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, const char * argv[]) {
    
    pid_t pid;
    printf("Main Process ID : %d\n\n", getpid());
    pid = fork();
    
    
    if(pid < 0) {
        fprintf(stderr, "Fork Failed");
        return 1;
    }
    else if(pid == 0){

        printf("Fork 1. I'm the child %d, my parent is %d.\n", getpid(), getppid());        //Child 1
        
        pid_t child_pid;
        for(int i=0; i<2; i++)
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
            printf("Fork 2. I'm the child %d, my parent is %d.\n", getpid(), getppid());        //Child 2
            pid_t grand_pid = fork();
            if(grand_pid == 0)
            {
                printf("Fork 3. I'm the child %d, my parent is %d.\n", getpid(), getppid());        //Child 3
                return 0;
            }
            else
            {
                waitpid(grand_pid, NULL, 0);
                return 0;
            }
        }
        else
        {
            waitpid(child_pid, NULL, 0);
            return 0;
        }
        
    }
    else {
        waitpid(pid, NULL, 0);
    }

    
    
    return 0;
}
