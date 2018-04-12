#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/signalfd.h>
#include <sys/socket.h>
#include <unistd.h>
#include <math.h>

#include <arpa/inet.h>
#include <linux/if.h>
#include <linux/if_tun.h>
#include <linux/ioctl.h>
#include <net/route.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "mac.h"
#include "reg.h"

#define DEBUG 3


// 1 == internal packet gen | 0 will use DMA
#define INTERNAL_PACKET_GEN 0

#define DELAY 10000

#define MTU 			80	//1564
#define HEADER_DATA_SIZE 	8
#define HEADER_FRAME_SIZE 	8 //16
#define CRC_SIZE			8
#define PADDING_SIZE		16//12
#define TX_BUF_SIZE			HEADER_DATA_SIZE + HEADER_FRAME_SIZE + MTU + PADDING_SIZE
#define RX_BUF_SIZE			HEADER_FRAME_SIZE + MTU + PADDING_SIZE + CRC_SIZE

//extern char 	*optarg;
static char 	tx_buffer[TX_BUF_SIZE];
static char 	rx_buffer[RX_BUF_SIZE];
//static struct 	pollfd pfd[2];

uint8_t reverseBits(uint8_t num)
{
    unsigned int  NO_OF_BITS = sizeof(num) * 8;
    unsigned int reverse_num = 0;
    int i;
    for (i = 0; i < NO_OF_BITS; i++) {
        if((num & (1 << i)))
            reverse_num |= 1 << ((NO_OF_BITS - 1) - i);
    }
    return reverse_num;
}

const char *byte_to_binary(int x)
{
    static char b[9];
    b[0] = '\0';

    int z;
    for (z = 128; z > 0; z >>= 1) {
        strcat(b, ((x & z) == z) ? "1" : "0");
    }

    return b;
}


static int setup_signal_handler(void)
{
    sigset_t mask;
    int ret;

    sigemptyset(&mask);
    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGPIPE);
    sigaddset(&mask, SIGHUP);
    sigaddset(&mask, SIGTERM);

    ret = sigprocmask(SIG_BLOCK, &mask, NULL);
    if (ret) {
        perror("Failed to setup signal mask");
        return -errno;
    }

    ret = signalfd(-1, &mask, 0);
    if (ret < 0) {
        perror("Failed to create signalfd");
        return -errno;
    }

    return ret;
}

static int receive_data()
{

    // Set packet Header info
    *(uint64_t*)(&tx_buffer[0]) = MTU + PADDING_SIZE + HEADER_FRAME_SIZE;
    *(uint64_t*)(&tx_buffer[HEADER_DATA_SIZE]) = 16;//ret;
    //*(uint64_t*)(&tx_buffer[HEADER_FRAME_SIZE]) = 16;//ret;

    // Fill data into packet
    int k=0;
    for (k=0; k<TX_BUF_SIZE - HEADER_FRAME_SIZE; k=k+1)
        *(uint64_t*)(&tx_buffer[k+HEADER_DATA_SIZE]) = k;

    // Debug
    printf("Header Data:\n");
    for(int i = 0; i < HEADER_DATA_SIZE + HEADER_FRAME_SIZE; i++)
        printf("%d, ", tx_buffer[i]);
    printf("\n");
    printf("Payload Data:\n");
    for(int i = HEADER_DATA_SIZE + HEADER_FRAME_SIZE; i < TX_BUF_SIZE; i++)
        printf("%d, ", tx_buffer[i]);
    printf("\n");

    // Send to IP
    modem_write((uint64_t*)tx_buffer, TX_BUF_SIZE, 0);

    return 0;
}

void *rx_thread_fnc(void* ptr)
{
    int i = 0;

    printf("TUN/TAP: Running Rx thread...\n");

    while(modem_running()) {
        modem_read((uint64_t*)rx_buffer, RX_BUF_SIZE);

        printf("-------Receiver Side-------\n");

        printf("Received Header Data\n");
        for(i = 0; i < HEADER_FRAME_SIZE+HEADER_FRAME_SIZE; i++)
            printf("%d, ", rx_buffer[i]);
        printf("\n");

        if(*(uint64_t*)&(rx_buffer[RX_BUF_SIZE - CRC_SIZE]))
            printf("RADIO: CRC ERROR\n");

        printf("Received Full Payload\n");
        for(i = 0; i < TX_BUF_SIZE; i++) {
            printf("%d, ", rx_buffer[i]);
        }
        printf("\n");

#if(INTERNAL_PACKET_GEN > 0)
        printf("|");
        // Use for internal packet generation print
        for(i = 0; i < RX_BUF_SIZE; i=i+8) {
            printf("%c", rx_buffer[i]);
        }
        printf("|\n");
#endif
        break;
    }

#if(INTERNAL_PACKET_GEN ==0)
    printf("\n---------RX|TX---------\n");
    for(i = 0; i < TX_BUF_SIZE; i++) {
        printf("%d %d\n", rx_buffer[i]>>0, tx_buffer[i+HEADER_DATA_SIZE]);
    }
#endif
    printf("TUN/TAP: Exiting Rx thread\n");

    return NULL;
}

int main(int argc, char *argv[])
{
    int ret;
    pthread_t rx_thread;

    ret = setup_signal_handler();
    if (ret < 0)
        return 1;


    ret = modem_setup();
    if(ret) {
        perror("TUN/TAP: Failed to setup modem");
        return ret;
    }

    ret = modem_reset();
    if(ret) {
        perror("TUN/TAP: Failed to reset modem");
        return 1;
    }

    ret = modem_start();
    if(ret) {
        perror("TUN/TAP: Failed to start modem");
        return 1;
    }

    ret = pthread_create(&rx_thread, NULL, rx_thread_fnc, NULL);
    if(ret) {
        perror("TUN/TAP: Error - pthread_create");
        return 1;
    }


    printf("Setup modem defaults\n");
    defaults(INTERNAL_PACKET_GEN);

    if (INTERNAL_PACKET_GEN)
    {
        sleep(1);
        printf("Sending 1 packet from internal IP\n");
        reg_write(0x120, 1);
        reg_write(0x120, 0);
    }
    else
    {
        sleep(1);
        printf("Hit any key to transmit custom packet with DMA\n");
        getchar();
        printf("Sending data\n");
        ret = receive_data();
        sleep(3);
    }

    modem_stop();
    pthread_join(rx_thread, NULL);
    modem_close();

    printf("Exiting TUN/TAP daemon\n");

    return 0;
}
