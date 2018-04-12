
/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>
#include <string.h>

/******************************************************************************/
/********************** Macros and Constants Definitions **********************/
/******************************************************************************/
/* Userspace Paths */
#define MODEM_UIO_DEV			"/dev/uio0"

/******************************************************************************/
/************************ Variables Definitions *******************************/
/******************************************************************************/
int			rx_dma_uio_fd;
void		*rx_dma_uio_addr;
uint32_t	rx_buff_mem_size;
uint32_t	rx_buff_mem_addr;

/***************************************************************************//**
 * @brief get_file_info
*******************************************************************************/
int32_t get_file_info(const char *filename, uint32_t *info)
{
	int32_t ret;
	FILE* fp;

	fp = fopen(filename,"r");
	if (!fp) {
		printf("%s: File %s cannot be opened.", __func__, filename);
		return -1;
	}
	ret = fscanf(fp,"0x%x", info);
	if (ret < 0) {
		printf("%s: Cannot read info from file %s.", __func__, filename);
		return -1;
	}
	fclose(fp);

	return 0;
}

/***************************************************************************//**
 * @brief uio_read
*******************************************************************************/
void uio_read(void *uio_addr, uint32_t reg_addr, uint32_t *data)
{
	*data = (*((unsigned *) (uio_addr + reg_addr)));
}

/***************************************************************************//**
 * @brief uio_write
*******************************************************************************/
void uio_write(void *uio_addr, uint32_t reg_addr, uint32_t data)
{
	*((unsigned *) (uio_addr + reg_addr)) = data;
}

/***************************************************************************//**
 * @brief uio_write
*******************************************************************************/
int32_t modem_write(uint32_t reg_addr, uint32_t data)
{
	int			modem_uio_fd_rx;
	void		*modem_uio_addr_rx;

	modem_uio_fd_rx = open(MODEM_UIO_DEV, O_RDWR);
	if(modem_uio_fd_rx < 1) {
		printf("%s: Can't open Rx modem_uio device\n\r", __func__);

		return modem_uio_fd_rx;
	}

	modem_uio_addr_rx = mmap(NULL,
			      4096,
			      PROT_READ|PROT_WRITE,
			      MAP_SHARED,
			      modem_uio_fd_rx,
			      0);

	uio_write(modem_uio_addr_rx, reg_addr, data);

	munmap(modem_uio_addr_rx, 4096);

	close(modem_uio_fd_rx);

	return 0;
}

/***************************************************************************//**
 * @brief uio_write
*******************************************************************************/
int32_t modem_read(uint32_t reg_addr, uint32_t *data)
{
	int			modem_uio_fd_rx;
	void		*modem_uio_addr_rx;

	modem_uio_fd_rx = open(MODEM_UIO_DEV, O_RDWR);
	if(modem_uio_fd_rx < 1) {
		printf("%s: Can't open Rx modem_uio device\n\r", __func__);

		return modem_uio_fd_rx;
	}

	modem_uio_addr_rx = mmap(NULL,
			      4096,
			      PROT_READ|PROT_WRITE,
			      MAP_SHARED,
			      modem_uio_fd_rx,
			      0);

	uio_read(modem_uio_addr_rx, reg_addr, data);

	munmap(modem_uio_addr_rx, 4096);

	close(modem_uio_fd_rx);

	return 0;
}

/***************************************************************************//**
* @brief main
*******************************************************************************/
int main(int argc, char *argv[])
{
	uint32_t data;
	int opt;
	
	printf("Configuring modem IP\n");
	
	while ((opt = getopt(argc, argv, "f:e:s:d:t:b:p:z:1:A:S:L:V:x")) != -1) {
        switch (opt) {
		case 'f': //FRLoopBw
			printf("Setting the FRLoopBw\n");
			data = atoi(optarg);
			modem_write(0x100, data); 
            break;
		case 'e': //EQmu
			printf("Setting the EQmu\n");
			data = atoi(optarg);
			modem_write(0x104, data); 
            break;
		case 's': //Scope select
			printf("Setting the Scope select\n");
			data = atoi(optarg);
			modem_write(0x108, data);  
            break;
		case 'd': //Debug selector
			printf("Setting the Debug selector\n");
			data = atoi(optarg);
			modem_write(0x10C, data);  
            break;			
		case 't': //Tx DMA select
			printf("Setting the Tx DMA select\n");
			data = atoi(optarg);
			modem_write(0x110, data);  
            break;
		case 'b': //EQ Bypass
			printf("Setting the EQ Bypass\n");
			data = atoi(optarg);
			modem_write(0x114, data);  
            break;
		case 'p': //RX Decode Enable
			printf("Setting the Enable RX\n");
			data = atoi(optarg);
			modem_write(0x118, data);  
            break;
		case 'z': //Packet Detection Threshold
			printf("Setting the Packet Detection Threshold\n");
			data = atoi(optarg);
			modem_write(0x11C, data);  
            break;
		case '1': //Send single packet
			printf("Sending single packet\n");
			modem_write(0x120, 1);  
			modem_write(0x120, 0);  
            break;
		case 'A': //Enable/Disable continuous packet transmit stream
			printf("Send Always Trigger\n");
			data = atoi(optarg);
			modem_write(0x124, data);  
            break;
		case 'S': //Set packet source [0=DMA, 1=Internal packet generator]
			printf("Setting Packet Source\n");
			data = atoi(optarg);
			modem_write(0x128, data);  
            break;
		case 'L': //Enable internal IP loopback
			printf("Setting Loopback Source\n");
			data = atoi(optarg);
			modem_write(0x12C, data);  
            break;
		case 'V': //Enable encoding
			printf("Disable channel coding\n");
			data = atoi(optarg);
			modem_write(0x130, data);//1==Coding Bypass
            break;
		case 'x':
			printf("Setting the defaults\n");
			modem_write(0x0, 0x1); //reset
			modem_write(0x100, (uint32_t)40); //FRLoopBw
			modem_write(0x104, (uint32_t)50); //EQmu
			modem_write(0x108, (uint32_t)2);  //Scope select
			//modem_write(0x108, (uint32_t)1);  //Scope select
			modem_write(0x110, (uint32_t)0);  //Tx DMA select
			modem_write(0x114, (uint32_t)0);  //EQ Bypass
			modem_write(0x118, (uint32_t)0);  //Rx Enable
			modem_write(0x11C, (uint32_t)10);  //PD Threshold
			modem_write(0x120, (uint32_t)0);  //Packet Toggle Transmit
			modem_write(0x124, (uint32_t)0);  //Packet Transmit Always
			modem_write(0x128, (uint32_t)0);  //Packet Source Select
			modem_write(0x12C, (uint32_t)0);  //Loopback control [0 Loopback, 1 RF]

			break;
		default: /* '?' */
            fprintf(stderr, "Usage: %s\n [-f FRLoopBw]\n [-e EQmu]\n [-s Scope select]\n [-d Debug Selector]\n [-t Tx DMA Select]\n [-b EQ Bypass]\n [-p Enable RX]\n [-L Loopback control]\n [-S Set transmitter source]\n [-A Toggle constant transmit mode]\n [-1 Send N packets in series]\n [-V Enable coding bypass]\n [-z Set packet detection threshold]\n [-x Default values]\n", argv[0]);
		}
    }

	/*	
	modem_read(0x12C, &data); //payloadLen
	printf("CRC Errors: %d\n", data);

	modem_read(0x12C, &data); //payloadLen
	printf("payloadLen: %d\n", data);


	modem_read(0x130, &data); //payloadLen
	printf("Packet Count: %d\n", data);

	//modem_read(0x128, &data); //payloadLen
	//printf("payloadLen: %d\n", data);

	// Cycle through debug registers

	//Timing Lock
	modem_write(0x10C, (uint32_t)1);
	modem_read(0x128, &data); //payloadLen
	printf("Timing Lock: %d\n", data);

	//Timing Lock
	modem_write(0x10C, (uint32_t)2);
	modem_read(0x128, &data); //payloadLen
	printf("Peaks Found: %d\n", data);

	//Timing Lock
	modem_write(0x10C, (uint32_t)3);
	modem_read(0x128, &data); //payloadLen
	printf("Frequency Lock: %d\n", data);

	//Header Failures
	modem_write(0x10C, (uint32_t)4);
	modem_read(0x128, &data); //payloadLen
	printf("Header Failures: %d\n", data);
	*/
	printf("All Done\n");
	
	return 0;
}
