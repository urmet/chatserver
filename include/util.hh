#include <streambuf>

// null streambuf for logging purposes

class nullbuf : public std::streambuf
{
public:
    virtual std::streamsize xsputn ( const char * , std::streamsize n ) override {
        // pretend like everything got written
        return n;
    }
    virtual int overflow ( int ) override {
        // indicate success, do nothing
        return 1;
    }
};
