
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

XUartPs UartPs;
XUartPs_Config *UartPs_Config;
XCodec Codec;
XBram_Config *Bram_Config;
XBram Bram;

void init_LBlock();



int main()
{
	init_platform();
	init_LBlock();
	xil_printf("finished");
	cleanup_platform();
	return 0;
}

void init_LBlock()
{
	int Status =0;
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

}
