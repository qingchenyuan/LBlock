/******************************************************************************
*
* Copyright (C) 2009 - 2014 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/

/*
 * helloworld.c: simple test application
 *
 * This application configures UART 16550 to baud rate 9600.
 * PS7 UART (Zynq) is not initialized by this application, since
 * bootrom/bsp configures it to baud rate 115200
 *
 * ------------------------------------------------
 * | UART TYPE   BAUD RATE                        |
 * ------------------------------------------------
 *   uartns550   9600
 *   uartlite    Configurable only in HW design
 *   ps7_uart    115200 (configured by bootrom/bsp)
 */

#include <stdio.h>
#include <stdlib.h>
#include "platform.h"
#include "xil_printf.h"
// FatFS addition part
#include "xsdps.h"
#include "ff.h"
#include "xil_cache.h"

int main()
{
    init_platform();
    FIL file1;
    FIL file2;
    FRESULT result;
    TCHAR *path = "0:/";
    FATFS *fs;
    char buff1[8];
    char buff2[8];
   // u64 mem_buff[10000];
    unsigned int count1,count2;
    xil_printf("the size of char is %d\n",sizeof(long long));
    fs = malloc(sizeof(FATFS));
    result = f_mount(fs,path,0);
    if (result !=0)
    {
    	xil_printf("error : f_mount\n");
    }
    result = f_open(&file1,"Text.txt",FA_READ);
    if(result != 0)
    {
      	xil_printf("error : f_open\n");
    }
    result = f_open(&file2,"Cipher.txt",FA_CREATE_ALWAYS|FA_WRITE);
    if(result != 0)
    {
      	xil_printf("error : f_open\n");
    }
    result = f_read(&file1,buff1,8,&count1);
    if(result != 0)
    {
      	xil_printf("error : f_read\n");
    }
    result = f_write(&file2, buff1, 8, &count2);
    if(result !=0)
    {
    	xil_printf("error : f_write\n");
    }
    f_close(&file1);
    f_close(&file2);
    cleanup_platform();
}
