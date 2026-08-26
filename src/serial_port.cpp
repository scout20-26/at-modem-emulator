#include "serial_port.h"

#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <stdexcept>
#include <string>
#include <system_error>
#include <termios.h>
#include <unistd.h>

namespace atmodem {
namespace {

speed_t baud_to_speed(int baud)
{
    switch (baud) {
    case 1200: return B1200;
    case 2400: return B2400;
    case 4800: return B4800;
    case 9600: return B9600;
    case 19200: return B19200;
    case 38400: return B38400;
    case 57600: return B57600;
    case 115200: return B115200;
    case 230400: return B230400;
    default:
        throw std::invalid_argument("unsupported baud rate: " +
                                    std::to_string(baud));
    }
}

[[noreturn]] void throw_errno(const char* operation)
{
    throw std::system_error(errno, std::generic_category(), operation);
}

} // namespace

SerialPort::~SerialPort()
{
    close();
}

void SerialPort::open(const std::string& device, int baud)
{
    close();

    const int new_fd = ::open(device.c_str(),
                              O_RDWR | O_NOCTTY | O_CLOEXEC);
    if (new_fd < 0) {
        throw_errno("open tty");
    }

    termios tty{};
    if (tcgetattr(new_fd, &tty) != 0) {
        const int saved_errno = errno;
        ::close(new_fd);
        errno = saved_errno;
        throw_errno("tcgetattr");
    }

    cfmakeraw(&tty);

    const speed_t speed = baud_to_speed(baud);
    if (cfsetispeed(&tty, speed) != 0 ||
        cfsetospeed(&tty, speed) != 0) {
        const int saved_errno = errno;
        ::close(new_fd);
        errno = saved_errno;
        throw_errno("cfset speed");
    }

    // 8 data bits, no parity, 1 stop bit.
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;
    tty.c_cflag &= ~PARENB;
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CRTSCTS;
    tty.c_cflag |= CLOCAL | CREAD;

    // Return one byte as soon as it arrives.
    tty.c_cc[VMIN] = 1;
    tty.c_cc[VTIME] = 0;

    if (tcsetattr(new_fd, TCSANOW, &tty) != 0) {
        const int saved_errno = errno;
        ::close(new_fd);
        errno = saved_errno;
        throw_errno("tcsetattr");
    }

    fd_ = new_fd;
}

void SerialPort::close() noexcept
{
    if (fd_ >= 0) {
        ::close(fd_);
        fd_ = -1;
    }
}

std::size_t SerialPort::read_some(char* buffer, std::size_t size)
{
    if (fd_ < 0) {
        throw std::logic_error("serial port is not open");
    }

    for (;;) {
        const ssize_t result = ::read(fd_, buffer, size);
        if (result >= 0) {
            return static_cast<std::size_t>(result);
        }

        if (errno == EINTR) {
            continue;
        }

        throw_errno("read tty");
    }
}

void SerialPort::write_all(const char* data, std::size_t size)
{
    if (fd_ < 0) {
        throw std::logic_error("serial port is not open");
    }

    std::size_t written = 0U;
    while (written < size) {
        const ssize_t result =
            ::write(fd_, data + written, size - written);

        if (result > 0) {
            written += static_cast<std::size_t>(result);
            continue;
        }

        if (result < 0 && errno == EINTR) {
            continue;
        }

        if (result == 0) {
            throw std::runtime_error("write tty returned zero");
        }

        throw_errno("write tty");
    }
}

void SerialPort::write_all(const std::string& data)
{
    write_all(data.data(), data.size());
}

} // namespace atmodem
