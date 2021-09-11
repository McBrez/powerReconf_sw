#include <xil_printf.h>
#include <xuartlite.h>
#include "microblaze_sleep.h"
#include "experiment.hpp"
#include <xhwicap.h>
#include <xspi.h>
#include <xgpio.h>
#include "command.hpp"
#include "xsysmon.h"

static XUartLite xUartLite;
static XSpi xSpi;
static XHwIcap HwIcap;
static XGpio Gpio;

#define BITSTREAM_P0_EMPTY_ADDR	0x003B0000U
#define BITSTREAM_P0_EMPTY_SIZE	287384U
#define BITSTREAM_P0_FULL_ADDR	0x00400000U
#define BITSTREAM_P0_FULL_SIZE	287384U
#define BITSTREAM_P1_EMPTY_ADDR	0x00450000U
#define BITSTREAM_P1_EMPTY_SIZE	263144U
#define BITSTREAM_P1_FULL_ADDR	0x00500000U
#define BITSTREAM_P1_FULL_SIZE	263144U
#define BITSTREAM_P2_EMPTY_ADDR	0x00550000U
#define BITSTREAM_P2_EMPTY_SIZE	287384U
#define BITSTREAM_P2_FULL_ADDR	0x00600000U
#define BITSTREAM_P2_FULL_SIZE	287384U
#define BITSTREAM_P3_EMPTY_ADDR	0x00650000U
#define BITSTREAM_P3_EMPTY_SIZE	263144U
#define BITSTREAM_P3_FULL_ADDR	0x00700000U
#define BITSTREAM_P3_FULL_SIZE	263144U

#define RECV_BUFFER_LEN 10
#define SEND_BUFFER_LEN 100

u32 composeOperand(u16 real, u16 imag)
{
	u32 imag_u32 = imag;

	return (imag_u32 << 16) + real;
}

void reconfigure(u32 frame, u32 configuration, XHwIcap* hwicap, UINTPTR spiBaseAddress) {
	UINTPTR bitstreamOffset0;
	UINTPTR bitstreamSize0;
	UINTPTR bitstreamOffset1;
	UINTPTR bitstreamSize1;

	switch(frame) {
	case 0:
		switch(configuration) {
		case 0:
			bitstreamOffset0 = BITSTREAM_P0_FULL_ADDR;
			bitstreamSize0 = BITSTREAM_P0_FULL_SIZE;
			bitstreamOffset1 = BITSTREAM_P1_EMPTY_ADDR;
			bitstreamSize1 = BITSTREAM_P1_EMPTY_SIZE;

			break;

		case 1:
			bitstreamOffset0 = BITSTREAM_P0_EMPTY_ADDR;
			bitstreamSize0 = BITSTREAM_P0_EMPTY_SIZE;
			bitstreamOffset1 = BITSTREAM_P1_FULL_ADDR;
			bitstreamSize1 = BITSTREAM_P1_FULL_SIZE;

			break;

		default:
			return;

		}
		break;

	case 1:
		switch(configuration) {
		case 0:
			bitstreamOffset0 = BITSTREAM_P2_FULL_ADDR;
			bitstreamSize0 = BITSTREAM_P2_FULL_SIZE;
			bitstreamOffset1 = BITSTREAM_P3_EMPTY_ADDR;
			bitstreamSize1 = BITSTREAM_P3_EMPTY_SIZE;

			break;

		case 1:
			bitstreamOffset0 = BITSTREAM_P2_EMPTY_ADDR;
			bitstreamSize0 = BITSTREAM_P2_EMPTY_SIZE;
			bitstreamOffset1 = BITSTREAM_P3_FULL_ADDR;
			bitstreamSize1 = BITSTREAM_P3_FULL_SIZE;

			break;

		default:
			return;

		}
		break;

	default:
		return;

	}

	for(unsigned int wordIndex = 0; wordIndex < bitstreamSize0/4 ; wordIndex++) {
		u32 address = spiBaseAddress + bitstreamOffset0 + wordIndex*4;
		u32 buffer = Xil_In32(address);
		XHwIcap_DeviceWrite(hwicap, &buffer, 1);
	}
	for(unsigned int wordIndex = 0; wordIndex < bitstreamSize1/4 ; wordIndex++) {
		u32 address = spiBaseAddress + bitstreamOffset1 + wordIndex*4;
		u32 buffer = Xil_In32(address);
		XHwIcap_DeviceWrite(hwicap, &buffer, 1);
	}
}

Command currentCommand;

int main()
{
	// Initialize peripherals.
	// UART
	XUartLite_Initialize(&xUartLite, XPAR_AXI_UARTLITE_0_DEVICE_ID);
	xil_printf("Board has been initialized.\n");

	experiment::Experiment experiment(XPAR_EXPERIMENTSYSTEM_0_BASEADDR,1);

	XSpi_Config *ConfigPtr;
	ConfigPtr = XSpi_LookupConfig(XPAR_AXI_QUAD_SPI_0_DEVICE_ID);
	if (ConfigPtr == NULL) {
		return XST_DEVICE_NOT_FOUND;
	}
	XSpi_CfgInitialize(&xSpi, ConfigPtr, ConfigPtr->BaseAddress);
	Xil_Out32(ConfigPtr->BaseAddress + 0x60, 0b00);

	XHwIcap_Config *CfgPtr;
	CfgPtr = XHwIcap_LookupConfig(XPAR_HWICAP_0_DEVICE_ID);
	if (CfgPtr == NULL) {
		return XST_FAILURE;
	}
	int status = XHwIcap_CfgInitialize(&HwIcap, CfgPtr, CfgPtr->BaseAddress);
	if(status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	status = XHwIcap_SelfTest(&HwIcap);
	if(status != XST_SUCCESS) {
			return XST_FAILURE;
	}

	/* Initialize the GPIO driver */
	XGpio_Initialize(&Gpio, XPAR_AXI_GPIO_0_DEVICE_ID);
	/* Set the direction for all signals as inputs except the LED output */
	XGpio_SetDataDirection(&Gpio, 1, 0xFFFFFFFF);

	u8* recv_buffer = new u8[RECV_BUFFER_LEN];
	for (int i = 0; i < RECV_BUFFER_LEN; i++) {
		recv_buffer[i] = 0;
	}
	u8* send_buffer = new u8[SEND_BUFFER_LEN];
	for (int i = 0; i < SEND_BUFFER_LEN; i++) {
		send_buffer[i] = 0;
	}

	// Init XADC
	XSysMon sysMonInst;
	XSysMon_Config *sysMonConfigPtr;
	sysMonConfigPtr = XSysMon_LookupConfig(XPAR_XADC_WIZ_0_DEVICE_ID);
	if (sysMonConfigPtr == NULL) {
		return XST_FAILURE;
	}
	XSysMon_CfgInitialize(&sysMonInst, sysMonConfigPtr,sysMonConfigPtr->BaseAddress);

	int Status = XSysMon_SelfTest(&sysMonInst);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	XSysMon_SetSeqChEnables(&sysMonInst, XSM_SEQ_CH_TEMP);
	XSysMon_SetAdcClkDivisor(&sysMonInst, 32);
	XSysMon_SetSequencerMode(&sysMonInst, XSM_SEQ_MODE_CONTINPASS);

	u32 operandX = composeOperand(4, 3);
	u32 operandY = composeOperand(2, 1);

	experiment.writeOperand(operandX, 0, experiment::OPERAND_IDX_X);
	experiment.writeOperand(operandY, 0, experiment::OPERAND_IDX_Y);

	MB_Sleep(1000);

	u32 resultR = experiment.readResult(0, experiment::RESULT_IDX_OUTR);
	u32 resultI = experiment.readResult(0, experiment::RESULT_IDX_OUTI);
	u32 resultOverflow = experiment.readResult(0, experiment::RESULT_IDX_OVERFLOW);

	xil_printf("Results: Real: %d\tImaginary: %d\tOverflow:%d", resultR, resultI, resultOverflow);

    return 0;
}
