//
//  main.cpp
//  virtual_memory_manager
//
//  Created by Henry on 25/12/2017.
//  Copyright © 2017 henry. All rights reserved.
//

#include <iostream>
#include <map>
#include <vector>
#include <fstream>
#include <string>
#include <math.h>
#include <vector>

using namespace std;
#define BACKEND_STORE "/Users/Henry/Downloads/OS_HW4_106Fall_v2/BACKING_STORE.bin"
#define ADDRESS_FILE "/Users/Henry/Downloads/OS_HW4_106Fall_v2/address.txt"
#define RESULT_FILE "/Users/Henry/Downloads/result.txt"
#define PAGE_SIZE 256

//Page Table
int Page_table[256];
//TLB Table
vector<pair<int, int>> TLB_table;
//Physical Memory
struct Page {
    char data[256];
}Page;
struct Page Physical_Memory[256];
int Physical_Memory_space_ptr = 0;


//TLB fetch
int TLB_fetch(int page_num) {
    int frame_num = -1;
    
    //find if page number exist in TLB
    for(int i = 0; i < TLB_table.size(); i++) {
        //find the page number
        if( page_num == TLB_table[i].first ) {
            frame_num = TLB_table[i].second;
            //resort the TLB table
            TLB_table.erase(TLB_table.begin()+i);
            TLB_table.push_back(make_pair(page_num, frame_num));
            break;
        }
    }
    
    return frame_num;
}

//TLB routine update
void TLB_update(bool TLB_hit, int page_num, int frame_num) {
    
    //if TLB don't hit, add the cache in
    if(!TLB_hit) {
        TLB_table.push_back(make_pair(page_num, frame_num));
    }
    
    //if TLB oversize 16, delete the oldest cache
    if(TLB_table.size() > 16) {
        TLB_table.erase(TLB_table.begin());
    }
    
    return ;
}

//Page table fetch
int Page_table_fetch(int page_num) {
    int frame_num = -1;
    
    //check if the frame_num exist
    if(Page_table[page_num] != -1) {    //page exist
        frame_num = Page_table[page_num];
    }
    
    return frame_num;
}

//Page Fault handler
int Backend_fetch(FILE *Backend_file, int page_num) {
    int frame_num = -1;
    
    //fetch the Backend File
    //get the position first
    ssize_t offset_position = page_num * PAGE_SIZE;
    fseek(Backend_file, offset_position, SEEK_SET);
    
    //read the 256-Byte in Physical Memory
    fread(Physical_Memory[Physical_Memory_space_ptr].data, sizeof(char), 256, Backend_file);
    frame_num = Physical_Memory_space_ptr;
    
    //update the Physical MM space pointer
    Physical_Memory_space_ptr++;
    
    //update the Page Table
    Page_table[page_num] = frame_num;
    
    return frame_num;
}

int Physical_Memory_fetch(int frame_num, int offset) {
    
    int data = Physical_Memory[frame_num].data[offset];

    return data;
}


void int_to_binary(int dec, int *binary) {
    int i = 0;
    while(dec > 0)
    {
        binary[i] = dec % 2;
        i++;
        dec = dec / 2;
    }
    for(i; i < 16; i++) {
        binary[i] = 0;
    }
}

int bin_to_pagenum(int *binary){
    int pagenum = 0;
    for(int i = 8; i < 16; i++){
        int base = pow(2, i - 8);
        pagenum += binary[i] * base;
    }
    return pagenum;
}

int bin_to_offset(int *binary){
    int offset = 0;
    for(int i = 0; i < 8; i++){
        int base = pow(2, i);
        offset += binary[i] * base;
    }
    return offset;
}

int bin_to_physical_address(int *physical_bin, int *logic_bin){
    int physical_address = 0;
    int combine_bin[16];
    
    //combine physical first 8 bits and logical first 8 bit
    for(int i = 0; i < 8; i++) {
        combine_bin[i] = logic_bin[i];
        combine_bin[i+8] = physical_bin[i];
    }
    //calculate the bin value
    for(int i = 0; i < 16; i++){
        int base = pow(2, i);
        physical_address += combine_bin[i] * base;
    }
    return physical_address;
}

int main(int argc, const char * argv[]) {
    
    FILE *Backend_file, *Address_file, *result_file;
    vector<string> input_array;
    int program_counter = 0;
    char address_file_buf[20];

    //open two file
    Backend_file = fopen(BACKEND_STORE, "rb");
    Address_file = fopen(ADDRESS_FILE, "r");
    result_file = fopen(RESULT_FILE, "w");
    
    //read first line of address file
    fgets(address_file_buf, 20 ,Address_file);
    //convert it into integer
    program_counter = atoi(address_file_buf);
    
    //clear the page table
    for(int i = 0; i < 256; i++) {
        Page_table[i] = -1;
    }

    //loop in the pc to simulate
    int logic_bin[16], physical_bin[16];
    int logic_address = 0;
    int physical_address = 0;
    int data_value = 0;
    int page_num = 0;
    int frame_num = 0;
    int offset = 0;
    int TLB_hit = 0;
    int Page_faults = 0;
    for(int pc = 0; pc < program_counter; pc++)
    {
        //read a line from file
        fgets(address_file_buf, 30 ,Address_file);
        //convert it into integer
        logic_address = atoi(address_file_buf);
        //conver int to binary, seperate it into pagenum and offset
        int_to_binary(logic_address, logic_bin);
        page_num = bin_to_pagenum(logic_bin);
        offset = bin_to_offset(logic_bin);
        
        //find TLB first
        bool TLB_hit_flag = 0;
        frame_num = TLB_fetch(page_num);
        
        //update TLB_hit
        if(frame_num != -1) {
            TLB_hit++;
            TLB_hit_flag = 1;
        }
        
        //if TLB return -1, go find Page Table
        if(frame_num == -1) {
            frame_num = Page_table_fetch(page_num);
        }
        
        //if Page Table return NULL, Page Fault, catch from BACKEND and save in Physical Memory
        if(frame_num == -1) {
            //update the Page_faults
            Page_faults++;
            //go to catch from BACKEND, update the Page table and Physical Memory
            frame_num = Backend_fetch(Backend_file, page_num);
        }
        
        //print physical address
        //translate the frame num to bin
        int_to_binary(frame_num, physical_bin);
        physical_address = bin_to_physical_address(physical_bin, logic_bin);
        //cout << physical_address << " ";
        
        //go physical memory to find data
        data_value = Physical_Memory_fetch(frame_num, offset);
        
        //print data
        //cout << data_value << endl;
        
        //put data into result file
        fprintf(result_file, "%d %d\n", physical_address, data_value);
        
        //update TLB table
        TLB_update(TLB_hit_flag, page_num, frame_num);
    }
    
    //print the TLB hits and Page Faults
    //cout << "TLB hits: " << TLB_hit << endl;
    //cout << "Page Faults: " << Page_faults << endl;
    
    //put TLB hits and Page Faults into result file
    fprintf(result_file, "TLB hits: %d\nPage Faults: %d\n", TLB_hit, Page_faults);
    
    //file close
    fclose(Backend_file);
    fclose(Address_file);
    fclose(result_file);
    
    return 0;
}
