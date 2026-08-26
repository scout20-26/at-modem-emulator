#include "config.h"

#include <cassert>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

int main()
{
    const std::string path = "test_modem.conf";

    {
        std::ofstream output(path);
        assert(output);
        output
            << "# comment\n"
            << "AT=OK\n"
            << "ATI=MyModem 1.0\\r\\nOK\\r\\n\n"
            << "AT+COPS*=+COPS: 0,0,\"TestOperator\"\\r\\nOK\\r\\n\n";
    }

    const atmodem::Config config = atmodem::Config::load(path);
    assert(config.rules().size() == 3U);

    assert(config.rules()[0].expect == "AT");
    assert(config.rules()[0].answer == "OK");

    assert(config.rules()[1].expect == "ATI");
    assert(config.rules()[1].answer == "MyModem 1.0\r\nOK\r\n");

    assert(config.rules()[2].expect == "AT+COPS*");
    assert(config.rules()[2].answer ==
           "+COPS: 0,0,\"TestOperator\"\r\nOK\r\n");

    std::remove(path.c_str());

    std::cout << "config tests: OK\n";
    return 0;
}
