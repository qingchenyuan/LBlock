
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include "platform.h"
#include "xcodec.h"
#include "xcodec_hw.h"
#include "xparameters.h"
#include "xtime_l.h"
#include "xil_printf.h"
#include "xstatus.h"
#include "xuartps.h"
#include "xuartps_hw.h"
#include "xbram.h"
#include "xsdps.h"
#include "ff.h"
#include "xil_cache.h"

XUartPs UartPs;
XUartPs_Config *UartPs_Config;
XCodec Codec;
XBram_Config *Bram_Config;
XBram Bram;
FATFS *fs;
FIL file1;
FIL file2;
FRESULT result;
TCHAR *Path = "0:/";


void init_LBlock();

//uint64_t mem_arr[1024];
u64 mem_arr[1024];// long long unsigned int, 64 bit,8 byte,1024行64bit，因为明文是64bit
// point to the BRAM Controllers(which controls the block rams)
u32 *BRAM1_mem = (u32 *)0x40000000;// 存储text的地方，0x后面8位，即8x4=32bit
u32 *BRAM2_mem = (u32 *)0x42000000;// 给了地址启示位，存储ans的地方
u64 mem_buff[10000];

int main()
{
	init_platform();
	/*init_LBlock();
	*BRAM1_mem = 0x10;
	*(BRAM1_mem+8)=0x11111111;
	*BRAM2_mem = 0xffffffff;
	xil_printf("finished");*/
	char a;
	int b;
	a = 'a';
	b = (unsigned int)a;
	xil_printf("the value is %d\n",b);
	cleanup_platform();
	return 0;
}
//测试字符强制转换，结果是字符a对应的97，运行结果是the value is 97

void init_LBlock()
{

	int Status = 0;
	UartPs_Config = XUartPs_LookupConfig(XPAR_XUARTPS_0_DEVICE_ID);
	if (UartPs_Config == NULL)
	{
		return XST_FAILURE;
	}
	Status = XUartPs_CfgInitialize(&UartPs, UartPs_Config,UartPs_Config->BaseAddress);
	if (Status != XST_SUCCESS)
	{
		return XST_FAILURE;
	}
	Status = XUartPs_SelfTest(&UartPs);
	if (Status != XST_SUCCESS)
	{
		return XST_FAILURE;
	}

	Status = XCodec_Initialize(&Codec,XPAR_CODEC_0_DEVICE_ID);
	if(Status != XST_SUCCESS)
	{
		xil_printf("Failed to initialized\n");
		return XST_FAILURE;
	}

	Bram_Config = XBram_LookupConfig(XPAR_AXI_BRAM_CTRL_0_DEVICE_ID);
	Status = XBram_CfgInitialize(&Bram, Bram_Config,Bram_Config->CtrlBaseAddress);
	if (Status != XST_SUCCESS)
	{
		return XST_FAILURE;
	}
	Status = XBram_SelfTest(&Bram, 0);
	if (Status != XST_SUCCESS)
	{
		return XST_FAILURE;
	}
	fs = malloc(sizeof(FATFS));
	result = f_mount(fs,Path,0);
	if (result != 0)
	{
		xil_printf("ERROR: f_mount failed %d\r\n", result);
		return XST_FAILURE;
	}
}
