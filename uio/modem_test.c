
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
#include <stdbool.h>
#include <iio.h>

/******************************************************************************/
/********************** Macros and Constants Definitions **********************/
/******************************************************************************/
/* Userspace Paths */
#define MODEM_UIO_DEV			"/dev/uio0"

//#define printf(...) printf("")

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

void config_transceiver(void)
{
        struct iio_context *ctx;
        struct iio_device *dev;
        struct iio_channel *chn;

        ctx = iio_create_default_context();
        dev = iio_context_find_device(ctx, "ad9361-phy");

        // RX
        chn = iio_device_find_channel(dev, "voltage0", false);
        iio_channel_attr_write_longlong(chn, "sampling_frequency", 20000000);
        iio_channel_attr_write_longlong(chn, "rf_bandwidth", 20000000);
        chn = iio_device_find_channel(dev, "altvoltage0", true);
        iio_channel_attr_write_longlong(chn, "frequency", 1900000000);

        // TX
        chn = iio_device_find_channel(dev, "voltage0", true);
        iio_channel_attr_write_longlong(chn, "sampling_frequency", 20000000);
        iio_channel_attr_write_longlong(chn, "rf_bandwidth", 20000000);
        chn = iio_device_find_channel(dev, "altvoltage1", true);
        iio_channel_attr_write_longlong(chn, "frequency", 1900000000);


        iio_context_destroy(ctx);
 }

void setup_modem(int source)
{
	// [source: 1==RF, 0==Internal]
	// This will test burst of N packets in IP loopback
	printf("Configuring modem IP\n");
	printf("Setting the defaults\n");
	modem_write(0x0, 0x1); //reset
	modem_write(0x118, (uint32_t)0);  //Rx Enable
	modem_write(0x100, (uint32_t)10); //FRLoopBw
	modem_write(0x104, (uint32_t)200); //EQmu
	modem_write(0x108, (uint32_t)1);  //Scope select
	modem_write(0x110, (uint32_t)0);  //Tx DMA select
	modem_write(0x114, (uint32_t)0);  //EQ Bypass
	modem_write(0x11C, (uint32_t)10);  //PD Threshold
	modem_write(0x120, (uint32_t)0);  //Packet Toggle Transmit
	modem_write(0x124, (uint32_t)0);  //Packet Transmit Always
	modem_write(0x128, (uint32_t)1);  //Packet Source Select
	modem_write(0x12C, (uint32_t)source);  //Loopback control [0=internal]
	modem_write(0x130, (uint32_t)0);  //Coding Bypass
	modem_write(0x134, (uint32_t)1);  //Viterbi Index
	modem_write(0x118, (uint32_t)1);  //Rx Enable

}


void send_packets(unsigned int delay, unsigned int data)
{
	unsigned int j=0;
	printf("Packet Sending Small burst (%d Packets)\n",data);
	for (j=0;j<data;j++) {
		modem_write(0x120, 1);  
		modem_write(0x120, 0);
		usleep(delay);
	}  
	printf("Packet Done Sending\n");
}


bool test_modem()
{

	uint32_t errors, rx, tx;

	// Direct statuses      
	modem_read(0x138, &errors);
	printf("CRC Errors: %d \n", errors);

	modem_read(0x13C, &rx);
	printf("Packet Count: %d \n", rx);

	modem_read(0x148, &tx);
	printf("TxPacket Count: %d \n", tx);

	return ((errors>0) || (tx!=rx));

}

/***************************************************************************//**
* @brief main
*******************************************************************************/
int main(void)
{
	uint32_t data, delay;

	data = 500;
	delay = 200;
	
	// Set up transceiver
	config_transceiver();

	// Configure modem IP
	setup_modem(0);
	// Send packets through generator
	send_packets(delay,data);
	if (test_modem())
		return -1;

	// Configure modem IP
	setup_modem(1);
	// Send packets through generator
	send_packets(delay,data);
	if (test_modem())
		return -1;
	
	printf("Passed all tests\n");
	return 0;
}
