#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

#include <iostream>
#include <fstream>

#include <ctime>
#include <RF24/RF24.h>

#define MAXServerResquest 1024
typedef uint8_t byte;

using namespace std;

RF24 radio(15,8); // radio(CE,CS)
byte addresses[][6] = {"50000"};

void radio_setup() {
  radio.begin();
  radio.setPALevel(RF24_PA_LOW);
  radio.openReadingPipe(1,addresses[0]);
  radio.printDetails();
  radio.startListening();
}
char buffer[32]="000";
char flag = 1;

void radio_read() {
  radio.read( buffer, sizeof(buffer) );
  cout << buffer << endl;
  sleep(1);
}

int main()
{
    int     f2s, s2f;                                       // fifo file descriptors
    char    serverRequest[MAXServerResquest];               // buffer for the request
    fd_set  rfds;                                           // flag for select
    struct  timeval tv;                                     // timeout
    tv.tv_sec = 1;                                          // 1 second
    tv.tv_usec = 0;                                         //

    radio_setup();
    do {
        radio_read();
        ofstream file;
        file.open("/tmp/data.csv", std::ios_base::app);
        if(flag == 1) flag = 0;       // la premiere valeur n'est pas significative alors on l'ecarte.
        else file << time(NULL) << "," << buffer << endl;
    }
    while (1);
    return 0;
}
