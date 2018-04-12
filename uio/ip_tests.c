
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
	unsigned int j=0;
	unsigned int coding = 0; // 1=Bypass channel coding
	
	printf("-------DON'T FORGET TO CONFIGURE RF-------\n");
	printf("Configuring modem IP\n");
	
	while ((opt = getopt(argc, argv, "R:O:B:L:h")) != -1) {
        switch (opt) {
		// This will test burst of N packets in RF loopback
		case 'R':
			printf("Setting the defaults\n");
			modem_write(0x0, 0x1); //reset
			modem_write(0x118, (uint32_t)0);  //Rx Enable [0=disable]
			modem_write(0x100, (uint32_t)10); //FRLoopBw
			modem_write(0x104, (uint32_t)200); //EQmu
			modem_write(0x108, (uint32_t)2);  //Scope select
			modem_write(0x110, (uint32_t)0);  //Tx DMA select [0=Modem,1=DMA]
			modem_write(0x114, (uint32_t)0);  //EQ Bypass
			modem_write(0x11C, (uint32_t)10);  //PD Threshold
			modem_write(0x120, (uint32_t)0);  //Packet Toggle Transmit
			modem_write(0x124, (uint32_t)0);  //Packet Transmit Always
			modem_write(0x128, (uint32_t)1);  //Packet Source Select
			modem_write(0x12C, (uint32_t)1);  //Loopback control [1=RF]

			modem_write(0x130, (uint32_t)coding);  //Coding Bypass
			//modem_write(0x134, (uint32_t)1);  //Viterbi Index

			printf("Sleeping before RX Enable\n");
			sleep(3);
			modem_write(0x118, (uint32_t)1);  //Rx Enable
			data = atoi(optarg);
			printf("Packet Sending Small burst (%d Packets)\n",data);
			for (j=0;j<data;j++) {
				modem_write(0x120, 1);  
				modem_write(0x120, 0);
				usleep(1000000);
			}  
			printf("Packet Done Sending\n");
            break;
		// This will test burst of N seconds of packets in RF loopback
		case 'O':
			printf("Setting the defaults\n");
			modem_write(0x0, 0x1); //reset
			modem_write(0x118, (uint32_t)0);  //Rx Enable [0=disable]
			modem_write(0x100, (uint32_t)10); //FRLoopBw
			modem_write(0x104, (uint32_t)200); //EQmu
			modem_write(0x108, (uint32_t)2);  //Scope select
			//modem_write(0x108, (uint32_t)1);  //Scope select
			modem_write(0x110, (uint32_t)0);  //Tx DMA select [0=Modem,1=DMA]
			modem_write(0x114, (uint32_t)0);  //EQ Bypass
			modem_write(0x11C, (uint32_t)10);  //PD Threshold
			modem_write(0x120, (uint32_t)0);  //Packet Toggle Transmit
			modem_write(0x124, (uint32_t)0);  //Packet Transmit Always
			modem_write(0x128, (uint32_t)1);  //Packet Source Select
			modem_write(0x12C, (uint32_t)1);  //Loopback control [1=RF]

			modem_write(0x130, (uint32_t)coding);  //Coding Bypass
			//modem_write(0x134, (uint32_t)1);  //Viterbi Index

			printf("Sleeping before RX Enable\n");
			sleep(3);
			modem_write(0x118, (uint32_t)1);  //Rx Enable
			printf("Sleeping before Packet Send\n");
			sleep(3);
			data = atoi(optarg);
			printf("Packet Sending (%d seconds)\n",data);
			modem_write(0x124, (uint32_t)1);  // Packet Send high rate
			sleep((unsigned int)data);
			modem_write(0x124, (uint32_t)0);  // Packet Send high rate
			printf("Packet Done Sending\n");
	break;
		// This will test burst of N packets in IP loopback
		case 'B':
			printf("Setting the defaults\n");
			modem_write(0x0, 0x1); //reset
			modem_write(0x118, (uint32_t)0);  //Rx Enable
			modem_write(0x100, (uint32_t)10); //FRLoopBw
			modem_write(0x104, (uint32_t)200); //EQmu
			modem_write(0x108, (uint32_t)3);  //Scope select
			//modem_write(0x108, (uint32_t)1);  //Scope select
			modem_write(0x110, (uint32_t)1);  //Tx DMA select
			modem_write(0x114, (uint32_t)0);  //EQ Bypass
			modem_write(0x11C, (uint32_t)10);  //PD Threshold
			modem_write(0x120, (uint32_t)0);  //Packet Toggle Transmit
			modem_write(0x124, (uint32_t)0);  //Packet Transmit Always
			modem_write(0x128, (uint32_t)1);  //Packet Source Select
			modem_write(0x12C, (uint32_t)0);  //Loopback control [0=internal]
			modem_write(0x130, (uint32_t)coding);  //Coding Bypass

			printf("Sleeping before RX Enable\n");
			sleep(3);
			modem_write(0x118, (uint32_t)1);  //Rx Enable
			data = atoi(optarg);
			printf("Packet Sending Small burst (%d Packets)\n",data);
			for (j=0;j<data;j++) {
				modem_write(0x120, 1);  
				modem_write(0x120, 0);
				usleep(1000000);
			}  
			printf("Packet Done Sending\n");
            break;
		// This will test burst of N seconds of packets in IP loopback
		case 'L':
			printf("Setting the defaults\n");
			modem_write(0x0, 0x1); //reset
			modem_write(0x118, (uint32_t)0);  //Rx Enable
			modem_write(0x100, (uint32_t)40); //FRLoopBw
			modem_write(0x104, (uint32_t)200); //EQmu
			modem_write(0x108, (uint32_t)2);  //Scope select
			//modem_write(0x108, (uint32_t)1);  //Scope select
			modem_write(0x110, (uint32_t)0);  //Tx DMA select
			modem_write(0x114, (uint32_t)0);  //EQ Bypass
			modem_write(0x11C, (uint32_t)10);  //PD Threshold
			modem_write(0x120, (uint32_t)0);  //Packet Toggle Transmit
			modem_write(0x124, (uint32_t)0);  //Packet Transmit Always
			modem_write(0x128, (uint32_t)1);  //Packet Source Select
			modem_write(0x12C, (uint32_t)0);  //Loopback control [0 = internal, 1 = RF]
			modem_write(0x130, (uint32_t)coding);  //Coding Bypass

			printf("Sleeping before RX Enable\n");
			sleep(3);
			modem_write(0x118, (uint32_t)1);  //Rx Enable
			printf("Sleeping before Packet Send\n");
			sleep(3);
			data = atoi(optarg);
			printf("Packet Sending (%d seconds)\n",data);
			modem_write(0x124, (uint32_t)1);  // Packet Send high rate
			sleep((unsigned int)data);
			modem_write(0x124, (uint32_t)0);  // Packet Send high rate
			printf("Packet Done Sending\n");
			break;
		default: /* '?' */
            fprintf(stderr, "Usage: %s\n [-L <seconds> Timed IP loopback]\n [-B <packets> Counted IP loopback]\n [-O <seconds> Timed RF loopback]\n [-R <packets> Counted RF loopback]\n", argv[0]);
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
