#include "modem.h"

#include "matcher.h"

#include <iostream>
#include <string_view>

namespace atmodem {

Modem::Modem(SerialPort& port, const Config& config)
    : port_(port), config_(config)
{
    input_.reserve(256U);
}

void Modem::send(const std::string& text)
{
    port_.write_all(text);
}

void Modem::process_command(std::string command)
{
    // Ignore an optional LF following CR.
    if (!command.empty() && command.back() == '\r') {
        command.pop_back();
    }

    if (command.empty()) {
        return;
    }

    std::cerr << "RX: " << command << '\n';

    for (const CommandRule& rule : config_.rules()) {
        if (Matcher::match(rule.expect, command)) {
            send(rule.answer);
            std::cerr << "TX: " << rule.answer << '\n';
            return;
        }
    }

    // Typical generic modem error for an unknown command.
    send("\r\nERROR\r\n");
    std::cerr << "TX: \\r\\nERROR\\r\\n\n";
}

void Modem::process_byte(char byte)
{
    // Echo characters exactly as received. CR/LF are also echoed.
    if (echo_enabled_) {
        port_.write_all(&byte, 1U);
    }

    if (byte == '\r') {
        process_command(input_);
        input_.clear();
        return;
    }

    if (byte == '\n') {
        // Accept LF-only terminals too. If CR was already processed, input
        // is empty and no duplicate command is generated.
        if (!input_.empty()) {
            process_command(input_);
            input_.clear();
        }
        return;
    }

    // Basic backspace support makes a serial terminal usable without
    // introducing a readline dependency.
    if (byte == '\b' || static_cast<unsigned char>(byte) == 0x7FU) {
        if (!input_.empty()) {
            input_.pop_back();
            if (echo_enabled_) {
                const char erase_sequence[] = "\b \b";
                send(erase_sequence);
            }
        }
        return;
    }

    input_.push_back(byte);

    // Prevent an accidental unlimited allocation if a broken client sends
    // data without a command terminator.
    constexpr std::size_t max_command_length = 4096U;
    if (input_.size() > max_command_length) {
        input_.clear();
        send("\r\nERROR\r\n");
    }

    // ATE0/ATE1 are special because they control the modem's echo state.
    // The response itself is still looked up in the configuration.
    if (input_ == "ATE0") {
        // Echo remains enabled until the complete command has been received.
        // The next command is therefore not echoed.
        echo_enabled_ = false;
    } else if (input_ == "ATE1") {
        echo_enabled_ = true;
    }
}

void Modem::run()
{
    char buffer[256];

    while (true) {
        const std::size_t count = port_.read_some(buffer, sizeof(buffer));

        for (std::size_t i = 0U; i < count; ++i) {
            process_byte(buffer[i]);
        }
    }
}

} // namespace atmodem
