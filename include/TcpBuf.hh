#ifndef TCPSTREAM_H
#define TCPSTREAM_H

#include <streambuf>
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>

// streambuf implementation for putting into regular old iostream
class TcpBuf : public std::streambuf
{
public:
    TcpBuf ( int sock_ ) : sock ( sock_ ) {
        std::clog<<"TcpBuf::TcpBuf("<<sock_<<")"<<std::endl;
        buf_pos = in_buf;
        end_pos = in_buf;
    }

    // delete copy constructor
    TcpBuf ( const TcpBuf& ) = delete;

    ~TcpBuf() {
        close ( sock );
    }

    // shutdown() causes the blocking read in to return early
    void stop() {
        shutdown ( sock, SHUT_RDWR );
    }


protected:
    virtual int_type overflow ( int_type __c = traits_type::eof() ) override;
    virtual int uflow() override;
    virtual int_type underflow() override;
    virtual std::streamsize xsputn ( const char* __s, std::streamsize __n ) override;

    virtual char* egptr() const {
        return end_pos;
    }

    virtual char* gptr() const {
        return buf_pos;
    }

    virtual void gbump ( int n ) {
        buf_pos += n;
    }

private:
    void strip_telnet_codes();
    char in_buf[100];
    char* buf_pos;
    char* end_pos;
    int sock;
};

#endif // TCPSTREAM_H
