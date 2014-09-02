#include "TcpBuf.hh"

using int_type = std::streambuf::int_type;

int_type TcpBuf::overflow ( int c )
{
    if ( c == traits_type::eof() ) {
        return traits_type::eof();
    }
    return write ( sock, &c, 1 );
}

int_type TcpBuf::uflow()
{
    if ( underflow() == traits_type::eof() ) {
        return traits_type::eof();
    }
    gbump ( 1 );
    return gptr() [-1];
}

int_type TcpBuf::underflow()
{
    if ( buf_pos < end_pos ) {
        return *buf_pos;
    } else { // buffer empty, read some
        int read_bytes = 0;
        read_bytes = read ( sock, in_buf, 99 );
        if ( read_bytes <= 0 )  {
            return traits_type::eof();
        }
        buf_pos = in_buf;
        end_pos = buf_pos + read_bytes;
        strip_telnet_codes(); // might clear the whole buffer
        if ( in_buf == end_pos ) {
            underflow();
        }
        return *buf_pos;
    }
}

std::streamsize TcpBuf::xsputn ( const char* __s, std::streamsize __n )
{
    return write ( sock, __s, __n );
}

void TcpBuf::strip_telnet_codes()
{
    static const unsigned char IAC = 255; // Interpret as command
    static const unsigned char SB = 250; // Subnegotiation start
    static const unsigned char SE = 240; // Subnegotiation end
    static const unsigned char CR = '\r';
    bool inSubnegotiation = false;
    char* read = buf_pos, *write = buf_pos;
    int commandBytesLeft = 0;

    while ( read < end_pos ) {
        unsigned char c = *read++;
        bool keep = true;
        switch ( c ) {
        case IAC:
            commandBytesLeft = 3;
            break;
        case SB:
            inSubnegotiation = true;
            break;
        case SE:
            inSubnegotiation = false;
            commandBytesLeft = 0;
            break;
        case CR:
            keep = false;
            break;
        }
        if ( commandBytesLeft > 0 ) {
            --commandBytesLeft;
            keep = false;
        }
        if ( inSubnegotiation ) {
            keep = false;
        }
        if ( keep ) {
            *write++ = c;    // strip out any telnet control/escape codes and CR
        }
    }
    end_pos = write;
}
