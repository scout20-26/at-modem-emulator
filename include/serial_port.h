#pragma once

#include <cstddef>
#include <string>

namespace atmodem {

class SerialPort {
public:
    SerialPort() = default;
    SerialPort(const SerialPort&) = delete;
    SerialPort& operator=(const SerialPort&) = delete;

    ~SerialPort();

    void open(const std::string& device, int baud);
    void close() noexcept;

    [[nodiscard]] int fd() const noexcept { return fd_; }

    std::size_t read_some(char* buffer, std::size_t size);
    void write_all(const char* data, std::size_t size);
    void write_all(const std::string& data);

private:
    int fd_{-1};
};

} // namespace atmodem
