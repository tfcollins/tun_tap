
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
int main(void)
{
	uint32_t data;

	struct reg registers[10];
	registers[0].name = "Timing Lock";
	registers[0].index = 1;
	registers[2].name = "Peaks Detected";
	registers[2].index = 2;
	registers[1].name = "Frequency Lock";
	registers[1].index = 3;
	registers[3].name = "Header Failures";
	registers[3].index = 4;

	int i = 1;
	initscr();
	while (1)
	{
		// Direct statuses	
		modem_read(0x138, &data); 
		printw("CRC Errors: %d \n", data);

		modem_read(0x13C, &data); 
		printw("Packet Count: %d \n", data);

		// Cycle through indexed debug registers
		int k;
		for (k=0;k<4;k++)
		{
			modem_write(0x10C, registers[k].index);
			modem_read(0x140, &data);
			printw("%s: %d \n", registers[k].name, data);
		}

		modem_read(0x144, &data); 
		printw("Last PayloadLen: %d \n", data);		

		modem_read(0x148, &data); 
		printw("TxPacket Count: %d \n", data);
		
		// Viterbi Pre Post
		modem_write(0x134, 1);
		modem_read(0x14C, &data); 
		printw("Viterbi Pre Word Errors: %d \n", data);
		modem_write(0x134, 0);
		modem_read(0x14C, &data); 
		printw("Viterbi Post Word Errors: %d \n", data);
		modem_write(0x134, 2);
		modem_read(0x14C, &data); 
		printw("Descrambler Post Word Errors: %d \n", data);
		
		printw("[Press Any Key To Stop]\n",i);
		refresh();
		timeout(500);
		i = getch();
		clear();
		if (i>=0)
			break;
	}
	endwin();

	printf("All Done\n");
	
	return 0;
}
