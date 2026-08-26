#pragma once

#include "config.h"
#include "serial_port.h"

#include <string>

namespace atmodem {

class Modem {
public:
    Modem(SerialPort& port, const Config& config);

    void run();

private:
    void process_byte(char byte);
    void process_command(std::string command);
    void send(const std::string& text);

    SerialPort& port_;
    const Config& config_;
    std::string input_;
    bool echo_enabled_{true};
};

} // namespace atmodem
