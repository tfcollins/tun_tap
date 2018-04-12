
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
#include <curses.h>

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

struct reg { char* name; uint32_t index;};

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

	struct reg registers[10];
	registers[0].name = "Timing Lock";
	registers[0].index = 1;
	registers[1].name = "Peaks Detected";
	registers[1].index = 2;
	registers[2].name = "Frequency Lock";
	registers[2].index = 3;
	registers[3].name = "Header Failures";
	registers[3].index = 4;



	WINDOW * mainwin;


	/*  Initialize ncurses  */

	if ( (mainwin = initscr()) == NULL ) {
	fprintf(stderr, "Error initialising ncurses.\n");
	exit(EXIT_FAILURE);
	}


	/*  Display "Hello, world!" in the centre of the
	screen, call refresh() to show our changes, and
	sleep() for a few seconds to get the full screen effect  */

	mvaddstr(13, 33, "Hello, world!");
	refresh();
	sleep(3);


	/*  Clean up after ourselves  */

	delwin(mainwin);
	endwin();
	refresh();



	printf("\n");
	
	while(1)
	{
	printf("\r");
	// Direct statuses	
	//modem_read(0x114, &data); //payloadLen
	//printf("payloadLen: %d | ", data);

	modem_read(0x12C, &data); //payloadLen
	printf("CRC Errors: %d | ", data);

	modem_read(0x130, &data); //payloadLen
	printf("Packet Count: %d | ", data);

	modem_read(0x138, &data); //payloadLen
	printf("payloadLen: %d | ", data);
	
	modem_read(0x13C, &data); //payloadLen
	printf("TxPacket Count: %d | ", data);

	// Cycle through indexed debug registers
	int k;
	for (k=0;k<4;k++)
	{
		modem_write(0x10C, registers[k].index);
		modem_read(0x134, &data);
		printf("%s: %d | ", registers[k].name, data);
	}
	fflush(stdout);	
	usleep(1000000);
	}
	printf("All Done\n");
	
	return 0;
}
