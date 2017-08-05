
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include "platform.h"
#include "xcodec.h"
#include "xcodec_hw.h"
#include "xparameters.h"

XCodec Codec;
//XCodec_Config *doCodec_cfg;

void init_LBlock()
{
	int status =0;
	status = XCodec_Initialize(&Codec,XPAR_CODEC_0_DEVICE_ID);
	if(status != XST_SUCCESS)
	{
		xil_printf("Failed to initialized\n");
		return XST_FAILURE;
	}

}
int main()
{
	init_platform();
	init_LBlock();
	xil_printf("finished");
	cleanup_platform();
	return 0;
}
