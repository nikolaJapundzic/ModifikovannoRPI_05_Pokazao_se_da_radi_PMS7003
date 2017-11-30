#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <bcm2835.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/if.h>
#include <curl/curl.h>

#include <time.h>

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/if.h>

#define DEBUG 1
//const char SERVER[] = {"srv.dunavnet.eu"};//const char SERVER[] = {"147.91.175.168"};//
//const char PORT[] = {"80"};//const char PORT[] = {"50034"};//
char mac[17] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
//int interval_var = INTERVAL;

const char WEBPATH[] = {"/rephandler/"};

char* mac_address()
{
  struct ifreq s;
  char temp[2] = { 0, 0 };
  int fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);

  strcpy(s.ifr_name, "eth0");
  if (0 == ioctl(fd, SIOCGIFHWADDR, &s)) 
  {
    int i;
    for (i = 0; i < 6; ++i)
	{
		if(DEBUG)
		printf(" %02x", (unsigned char) s.ifr_addr.sa_data[i]);
		sprintf(temp, "%02x " , (unsigned char)s.ifr_addr.sa_data[i]);
		strcat(mac, temp);
	}	
	puts("\n");
    return mac;
  }
  return mac;
}


int main(int argc, char ** argv) {
    
    	CURL *curl;
	CURLcode res2;
    
    	char buf[100];
	char* bre = mac_address();
	char buffer[1256] ;
    
	int slanje = 0;
    
	int PM25 = 0;
	int PM10 = 0;
	int PM1 = 0;
    
    	int n=0;
    
    
while(1)
{
  int fd;
  // Open the Port. We want read/write, no "controlling tty" status, and open it no matter what state DCD is in
  fd = open("/dev/ttyAMA0", O_RDWR | O_NOCTTY | O_NDELAY);
  if (fd == -1) {
    perror("open_port: Unable to open /dev/ttyAMA0 - ");
    return(-1);
    }
 
  struct termios options;
	tcgetattr(fd, &options);
	options.c_cflag = B9600 | CS8 | CLOCAL | CREAD;		//<Set baud rate
	options.c_iflag = IGNPAR;
	options.c_oflag = 0;
	options.c_lflag = 0;
	tcflush(fd, TCIFLUSH);
	tcsetattr(fd, TCSANOW, &options);

  // Turn off blocking for reads, use (fd, F_SETFL, FNDELAY) if you want that
  fcntl(fd, F_SETFL, 0);

  // Write to the port
  // int n = write(fd,"Hello",6);// int n = write("Hello Peripheral\n");
  /*if (n < 0) {
    perror("Write failed - ");
    return -1;
  }
  

  // Read up to 6 characters from the port if they are there

 
  */
 
    printf("\nReading data...\n");
    for(int k=0;k<50;k++)
      buf[k] = 0;
    bcm2835_delay(500);
	n = read(fd, (void*)buf, 50);
	bcm2835_delay(500);
    printf("n je: %d\n ", n);
    close(fd);
   
  if (n < 0) {
    perror("Read failed - ");
    return -1;
  } 
  else if (n == 0) 
	printf("No data on port\n");
  
  else if (buf[0]== 0x42  && buf[2] == 0x4d)
   {
      puts("NASAO 42 4D");
      for(int j=0;j<100;j++)
      	printf("bytes read : %x", buf[j]);
      
      /*PM1= ((buf[3] *256) + buf[2])/10; 
      printf(" PM25 : %d\n ", PM25);
      
      PM25= ((buf[3] *256) + buf[2])/10; 
      printf(" PM25 : %d\n ", PM25);
      
      PM10= ((buf[5] *256) + buf[4])/10; 
      printf(" PM10 : %d\n ", PM10);
      
      slanje = 1;*/
   }
   else {
        puts("NISAM NASAO 42 4D");
        for(int j=0;j<100;j++)
      	printf("bytes read : %x", buf[j]);
   }
    
   	//Build data payload
	const char DeviceName[] = { "ekoNETPMsensor" };
	const char DeviceSI[] = { "device si RPi" };
	const char gps[] = { ",,,,,,,,,," };
    
    if (slanje == 1) {
    slanje = 0;
    
	int PM_ON=1;

    

	sprintf(buffer, "{\"n\":\"%s\",\"ei\":\"%s\",\"si\":\"%s\",\"PM\":{\"on\":%d,\"PM1\":%d,\"PM25\":%d,\"PM10\":%d},\"gps\":\"%s\"}", DeviceName, bre, DeviceSI,PM_ON, PM1,PM25,PM10, gps);
              	  
	printf("Payload:  %s\n", buffer);

	curl_global_init(CURL_GLOBAL_DEFAULT);

	curl = curl_easy_init();
	 if (curl) {
               struct curl_slist *chunk = NULL;
               /* Add a custom header */
               chunk = curl_slist_append(chunk, "Content-Type: application/atom+xml;type=entry;charset=utf-8");
               //chunk = curl_slist_append(chunk, "Authorization: SharedAccessSignature sr=ekonetiothub.azure-devices.net&sig=8EY9rR6%2b%2bl8XjshjH0BSuPkefZCxHdqr2lT5z%2fTI2pE%3d&se=1487846951&skn=iothubowner");
               chunk = curl_slist_append(chunk, "Authorization: SharedAccessSignature sr=ekonetiothub.azure-devices.net&sig=7dMdCqh0DlBP0FiRNMkidX6Ic5dlbFAwRCQKK9o751Y%3D&se=1645886927&skn=deviceSend");

               /* set our custom set of headers */
               res2 = curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);

               curl_easy_setopt(curl, CURLOPT_POSTFIELDS, buffer);
               curl_easy_setopt(curl, CURLOPT_URL, "https://ekonetiothub.azure-devices.net/devices/ekonetDevicePM/messages/events?api-version=2016-02-03");
  #ifdef SKIP_PEER_VERIFICATION
		/*
		 * If you want to connect to a site who isn't using a certificate that is
		 * signed by one of the certs in the CA bundle you have, you can skip the
		 * verification of the server's certificate. This makes the connection
		 * A LOT LESS SECURE.
		 *
		 * If you have a CA cert for the server stored someplace else than in the
		 * default bundle, then the CURLOPT_CAPATH option might come handy for
		 * you.
		 */
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
#endif

#ifdef SKIP_HOSTNAME_VERIFICATION
		/*
		 * If the site you're connecting to uses a different host name that what
		 * they have mentioned in their server certificate's commonName (or
		 * subjectAltName) fields, libcurl will refuse to connect. You can skip
		 * this check, but this will make the connection less secure.
		 */
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
#endif

		/* Perform the request, res will get the return code */
		res2 = curl_easy_perform(curl);
		/* Check for errors */
		if (res2 != CURLE_OK)
			fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res2));

		if (DEBUG)
		{
			long http_code = 0;
			curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
			printf("Response status: %d\n", (int)http_code);
		}

		/* always cleanup */
		curl_easy_cleanup(curl);
     }
     
        curl_global_cleanup(); 
	
	} // slanje!!
  bcm2835_delay(15000);
  }
  return 0;
}
