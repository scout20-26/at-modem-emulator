#include "config.h"
#include "modem.h"
#include "serial_port.h"

#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {

struct Options {
    std::string device;
    std::string config;
    int baud = 115200;
};

void print_usage(const char* program)
{
    std::cerr
        << "Usage: " << program
        << " --device <tty> --config <file> [--baud <rate>]\n\n"
        << "Example:\n"
        << "  " << program
        << " -d /dev/ttyUSB0 -c config/modem.conf\n";
}

int parse_baud(const std::string& value)
{
    std::size_t consumed = 0U;
    const int baud = std::stoi(value, &consumed);
    if (consumed != value.size() || baud <= 0) {
        throw std::invalid_argument("invalid baud rate: " + value);
    }
    return baud;
}

Options parse_args(int argc, char* argv[])
{
    Options options;

    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];

        auto require_value = [&](const char* name) -> std::string {
            if (i + 1 >= argc) {
                throw std::invalid_argument(
                    std::string("missing value for ") + name);
            }
            ++i;
            return argv[i];
        };

        if (arg == "-d" || arg == "--device") {
            options.device = require_value(arg.c_str());
        } else if (arg == "-c" || arg == "--config") {
            options.config = require_value(arg.c_str());
        } else if (arg == "-b" || arg == "--baud") {
            options.baud = parse_baud(require_value(arg.c_str()));
        } else if (arg == "-h" || arg == "--help") {
            print_usage(argv[0]);
            std::exit(EXIT_SUCCESS);
        } else {
            throw std::invalid_argument("unknown option: " + arg);
        }
    }

    if (options.device.empty() || options.config.empty()) {
        throw std::invalid_argument("device and config are required");
    }

    return options;
}

} // namespace

int main(int argc, char* argv[])
{
    try {
        const Options options = parse_args(argc, argv);

        const atmodem::Config config =
            atmodem::Config::load(options.config);

        atmodem::SerialPort port;
        port.open(options.device, options.baud);

        std::cerr << "AT modem emulator started\n"
                  << "  device: " << options.device << '\n'
                  << "  baud:   " << options.baud << '\n'
                  << "  rules:  " << config.rules().size() << '\n';

        atmodem::Modem modem(port, config);
        modem.run();

        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        std::cerr << "error: " << error.what() << '\n';
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }
}
