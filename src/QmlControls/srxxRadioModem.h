#include <sys/types.h>
#include <cstdint>
#include <string>

class iRadioModem
{
public:
    virtual int init(int baud_rate) = 0;
    virtual ssize_t send_message(const char* msg, size_t length) = 0;
    virtual ssize_t read_serial(uint8_t* readbuffer, size_t numbytes, long int delay_time) = 0;
    virtual void setSerialDeviceName(const std::string& tty) = 0;
};

iRadioModem* getRadioModem();
