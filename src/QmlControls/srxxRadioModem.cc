#include "srxxRadioModem.h"

#include <string>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <sys/time.h>

class rfd900Modem : public iRadioModem
{
public:
    static constexpr int DEFAULT_BAUD_RATE = 57600;
    rfd900Modem()
    {
        baudRate = 0;
        serialfd = -1;
        //serialDeviceName = "/dev/ttyUSB1";
        if(init(57600) != 0){
            fprintf(stderr, "error, %s radio init failure\n", __func__);
        }
    }

    ~rfd900Modem()
    {
        if(serialfd != -1){
            close_serial();
        }
    }

    int init(int baud_rate)
    {
        serialfd = initialize_serial(baud_rate);
        if(serialfd == -1){
            return -1;
        }

        return 0;
    }

    void setSerialDeviceName(const std::string& tty)
    {
        serialDeviceName = std::string("/dev/" + tty);
    }

    ssize_t send_message(const char* msg, size_t length)
    {
        ssize_t totalBytesSent = 0;
        ssize_t bytesSent;
        ssize_t bytesRemaining = length - totalBytesSent;

        while(bytesRemaining > 0){
            bytesSent = write(serialfd, &msg[totalBytesSent], bytesRemaining);

            if(bytesSent >= 0){
                totalBytesSent += bytesSent;
            }
            else{
                fprintf(stderr, "error: %s, bytesSent: %ld, errno: %s\n", __func__, bytesSent, strerror(errno));
                break;
            }

            bytesRemaining = length - totalBytesSent;
        }

        return totalBytesSent;
    }

    ssize_t read_serial(uint8_t* readbuffer, size_t numbytes, long int delay_time)
    {
        int rv;                                             // return value
        struct timeval  timeout;
        fd_set read_set;                                    // file descriptor set

        timeout.tv_sec = delay_time / 1000000L;             // one sec in units of microseconds
        timeout.tv_usec = delay_time % 1000000L;

        FD_ZERO (&read_set);                       // clears the set
        FD_SET(serialfd, &read_set);         // adds the file descriptor to the set

        rv = select(serialfd+1, &read_set, NULL, NULL, &timeout);

        if(rv > 0){
            if(FD_ISSET(serialfd, &read_set))    // input from source available
            {
                memset(readbuffer, 0, numbytes);
                return read(serialfd,readbuffer,numbytes);
            }
            else{
                fprintf(stderr, "warn: %s, select returned %d, but FD_ISSET was false, serialfd: %d\n",
                        __func__, rv, serialfd);

            }
        }
        else if(rv < 0){
            fprintf(stderr, "%s, error on select - %s\n", __FUNCTION__, strerror(errno));
            fprintf(stderr, "select return value: %d\n", rv);
            return -1;
        }
        return 0;
    }

private:

    int initialize_serial(int baud_rate)
    {
        int serial_port_fd;
        int serial_speed = set_baud_speed(baud_rate);
        struct termios newtio;

        // Open the serial port nonblocking (read returns immediately)
        serial_port_fd = open(serialDeviceName.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK | O_SYNC);
        if(serial_port_fd < 0)             // open returns -1 on error
        {
            fprintf(stderr, "Error: %s, serial port not open, errno -%s\n", __FUNCTION__, strerror(errno));
            fprintf(stderr, "serial device name: %s\n", serialDeviceName.c_str());
            return -1;
        }

        else    fprintf(stderr, "success, open serial port fd = %d\n", serial_port_fd);

        // New port settings
        memset(&newtio, 0, sizeof(newtio));           // set all struct values to zero
        newtio.c_cflag = serial_speed | CS8 | CLOCAL | CREAD; //  | IGNBRK;
        newtio.c_iflag = IGNPAR | ICRNL;
        newtio.c_oflag = 0;                     // raw output
        newtio.c_lflag = ~ICANON;
        newtio.c_cc[VMIN] = 1;
        newtio.c_cc[VTIME] = 0;

        // Load new settings
        if( tcsetattr(serial_port_fd, TCSAFLUSH, &newtio) < 0){
            fprintf(stderr, "error, %s, set term attributes: %s\n", __func__, strerror(errno));
            close_serial();
            return -1;
        }
        usleep(10000);                                  // 10 ms
        tcflush(serial_port_fd, TCIOFLUSH);
        fprintf(stderr, "info, %s success\n", __func__);

        return serial_port_fd;
    }

    int set_baud_speed(int baud_rate)
    {
        switch(baud_rate)
        {
        case 9600:
            return B9600;
        case 19200:
            return B19200;
        case 38400:
            return B38400;
        case 57600:
            return B57600;
        case 115200:
            return B115200;
        default:
            fprintf(stderr, "warning: %s, argument baud_rate: %d not supported, setting to default %d\n",
                        __func__, baud_rate, DEFAULT_BAUD_RATE);
            return DEFAULT_BAUD_RATE;
        }
    }

    void close_serial()
    {
        if( close(serialfd) != -1){
                serialfd = -1;
        }
        else
        {
            fprintf(stderr, "%s, serial close error: %s\n", __func__, strerror(errno));
        }
    }

    int serialfd;               // serial file descriptor
    int baudRate;
    std::string serialDeviceName;
};

iRadioModem* getRadioModem()
{
    static rfd900Modem radio;
    return &radio;
}
