/* This is a simple application for testing SPI communication on a RedPitaya
** Test GPIO port P
*/
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>
#include <linux/spi/spidev.h>
#include <linux/types.h>
#include <sys/mman.h>

/* Inline functions definition */
static int init_spi();
static int release_spi();
static int write_spi(int* write_data, int size);
int write_gpio(long number);
unsigned int user_input(void);

/* Constants definition */
int spi_fd = -1;

int main(void){

    /* Sample data */
    //char *data = "REDPITAYA SPI TEST";	
	unsigned int option = 0;
	int data [6][1] = {{0x00000000}, {0x00000000}, {0x00000000}, {0x00000000}, {0x00000000}, {0x00000000}}; // Just initialize with all zero value
	int dataA [6][1] = {{0x18802c00}, {0x29800008}, {0x424e0000}, {0xb3040000}, {0x3c80ec00}, {0x05005800}}; //for 35Mhz
	int dataB [6][1] = {{0x00004000}, {0x11800008}, {0x424e0000}, {0xb3040000}, {0x3c80ac00}, {0x05005800}}; //for 800Mhz
        int dataC [6][1] = {{0x00005000}, {0x11800008}, {0x424e0000}, {0xb3040000}, {0x3c80ac00}, {0x05005800}}; //for 1000Mhz
        int dataD [6][1] = {{0x00003c00}, {0x11800008}, {0x424e0000}, {0xb3040000}, {0x3c809c00}, {0x05005800}}; //for 1.5Ghz
        int dataE [6][1] = {{0x00004800}, {0x11800008}, {0x424e0000}, {0xb3040000}, {0x3c809c00}, {0x05005800}}; //for 1.8Ghz
        int dataF [6][1] = {{0x00005000}, {0x11800008}, {0x424e0000}, {0xb3040000}, {0x3c809c00}, {0x05005800}}; //for 2.0Ghz
        int dataG [6][1] = {{0x00002c00}, {0x11800008}, {0x424e0000}, {0xb3040000}, {0x3c808c00}, {0x05005800}}; //for 2.2Ghz
        int dataH [6][1] = {{0x00003200}, {0x11800008}, {0x424e0000}, {0xb3040000}, {0x3c808c00}, {0x05005800}}; //for 2.5Ghz
        int dataI [6][1] = {{0x00003800}, {0x11800008}, {0x424e0000}, {0xb3040000}, {0x3c808c00}, {0x05005800}};  //for 2.8Ghz
        int dataJ [6][1] = {{0x00003c00}, {0x11800008}, {0x424e0000}, {0xb3040000}, {0x3c808c00}, {0x05005800}}; //for 3.0Ghz
        int dataK [6][1] = {{0x00004600}, {0x11800008}, {0x424e0000}, {0xb3040000}, {0x3c808c00}, {0x05005800}}; //for 3.5Ghz
        int dataL [6][1] = {{0x00004c00}, {0x11800008}, {0x424e0000}, {0xb3040000}, {0x3c808c00}, {0x05005800}}; //for 3.8Ghz
        int dataM [6][1] = {{0x00005000}, {0x11800008}, {0x424e0000}, {0xb3040000}, {0x3c808c00}, {0x05005800}}; //for 4.0Ghz
        int dataN [6][1] = {{0x00005400}, {0x11800008}, {0x424e0000}, {0xb3040000}, {0x3c808c00}, {0x05005800}}; //for 4.2Ghz
        int dataO [6][1] = {{0x00005800}, {0x11800008}, {0x424e0000}, {0xb3040000}, {0x3c808c00}, {0x05005800}}; //for 4.4Ghz
	
	

	
	if(init_spi() < 0){
            printf("Initialization of SPI failed. Error: %s\n", strerror(errno));
            return -1;
        }
	
	/*******  User added code block     **********/ 
	/* get User Input from terminal */
	do{
		option = user_input();
		
		switch(option){
			case 1 : memcpy(data, dataA, sizeof(dataA)); break;
			case 2 : memcpy(data, dataB, sizeof(dataB)); break;
			case 3 : memcpy(data, dataC, sizeof(dataC)); break;
			case 4 : memcpy(data, dataD, sizeof(dataD)); break;
			case 5 : memcpy(data, dataE, sizeof(dataE)); break;
			case 6 : memcpy(data, dataF, sizeof(dataF)); break;
			case 7 : memcpy(data, dataG, sizeof(dataG)); break;
			case 8 : memcpy(data, dataH, sizeof(dataH)); break;
			case 9 : memcpy(data, dataI, sizeof(dataI)); break;
			case 10 : memcpy(data, dataJ, sizeof(dataJ)); break;
			case 11 : memcpy(data, dataK, sizeof(dataK)); break;
			case 12 : memcpy(data, dataL, sizeof(dataL)); break;
			case 13 : memcpy(data, dataM, sizeof(dataM)); break;
			case 14 : memcpy(data, dataN, sizeof(dataN)); break;
			case 15 : memcpy(data, dataO, sizeof(dataO)); break;
			case 99 : return 0;
			default: printf("Wrong input!\n"); continue;
		}
		
		printf("Sending register data ...\n");
		/* Write R0~R5 data */
		for(int i=0; i<6; ++i){
			if(write_spi(data[i], sizeof(data[i])) < 0){
				printf("Write data[%i] to SPI failed. Error: %s\n", i, strerror(errno));
				return -1;
			}
		}
		printf("Send register data complete.\n");
		usleep(200000);			// Optional delay
	} while(option != 99);
	/****** End of user code block      *********/
	
	/* Release resources */
        if(release_spi() < 0){
            printf("Relase of SPI resources failed, Error: %s\n", strerror(errno));
            return -1;
        } 

    return 0;
}

static int init_spi(){

    /* MODES: mode |= SPI_LOOP;
    *        mode |= SPI_CPHA;
    *        mode |= SPI_CPOL;
    *                 mode |= SPI_LSB_FIRST;
    *        mode |= SPI_CS_HIGH;
    *        mode |= SPI_3WIRE;
    *        mode |= SPI_NO_CS;
    *        mode |= SPI_READY;
    *
    * multiple possibilities possible using | */
    int mode = 0;
    //mode |= SPI_MODE_2;

    /* Opening file stream */
    spi_fd = open("/dev/spidev1.0", O_RDWR | O_NOCTTY);

    if(spi_fd < 0){
        printf("Error opening spidev0.1. Error: %s\n", strerror(errno));
        return -1;
    }

    /* Setting mode (CPHA, CPOL) */
    if(ioctl(spi_fd, SPI_IOC_WR_MODE, &mode) < 0){
        printf("Error setting SPI_IOC_RD_MODE. Error: %s\n", strerror(errno));
        return -1;
    }

    /* Setting SPI bus speed */
    int spi_speed = 1000000;

    if(ioctl(spi_fd, SPI_IOC_WR_MAX_SPEED_HZ, &spi_speed) < 0){
        printf("Error setting SPI_IOC_WR_MAX_SPEED_HZ. Error: %s\n", strerror(errno));
        return -1;
    }

    return 0;
}

static int release_spi(){

    /* Release the spi resources */
    close(spi_fd);

    return 0;
}

/* Write data to the SPI bus */
static int write_spi(int* write_buffer, int size){

    int errmsg = write_gpio(0);

    int write_spi = write(spi_fd, write_buffer, size);

    if(write_spi < 0){
        printf("Failed to write to SPI. Error: %s\n", strerror(errno));
        return -1;
    }
	
	//set HIGH the GPIO 
	errmsg = write_gpio(16);
	errmsg = write_gpio(0);
	
	usleep(200);
	
    return 0;
}

/*  open the file and write into gpio  */
int write_gpio(long number)
{
  int fd_gpio;
  //char *end;      //|| end == argv[1] 
  volatile void *cfg;
  //long number;

  errno = 0;
  //number = (argc == 2) ? strtol(argv[1], &end, 10) : -1;
  if(errno != 0 || number < 0 || number > 255)
  {
    fprintf(stderr, "Usage: gpio-output [0-255]\n");
    return EXIT_FAILURE;
  }

  if((fd_gpio = open("/dev/mem", O_RDWR)) < 0)
  {
    fprintf(stderr, "Cannot open /dev/mem.\n");
    return EXIT_FAILURE;
  }

  cfg = mmap(NULL, sysconf(_SC_PAGESIZE), PROT_READ|PROT_WRITE, MAP_SHARED, fd_gpio, 0x40001000);

  *(uint8_t *)(cfg + 2) = (uint8_t)number;
  
  
  /* Clean the variable */
  close(fd_gpio);

  return EXIT_SUCCESS;
}

/*  Ask for user input  */
unsigned int user_input(void){
	
	unsigned int usr_inp = 0;
	unsigned int show_freq[15] = {35, 800, 1000, 1500, 1800, 2000, 2200, 2500, 2800, 3000, 3500, 3800, 4000, 4200, 4400};
	
	printf("Choose from the following option: (1~15)\n");
	for (int i = 0; i < 14; i++) {
		printf("\t%d. For %d MHz frequency\n", (i+1), show_freq[i]);
	}
	printf("For exit the program, Enter: 99.\n");
	
	printf("Enter input: ");
	scanf("%d", &usr_inp);
	
	return usr_inp;
}
