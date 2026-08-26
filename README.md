# AT Modem Emulator

Linux/C++20 test assignment: a small server that listens on a TTY device and
responds to AT commands using rules stored in a text configuration file.

The implementation deliberately does **not** use a regular-expression library.
The required mini-language is implemented in `Matcher`.

## Requirements

- Linux
- C++20 compiler
- CMake >= 3.16
- POSIX `termios`
- no external runtime dependencies

## Supported pattern syntax

| Syntax | Meaning |
|---|---|
| `.` | exactly one arbitrary character |
| `*` | zero or more arbitrary characters |
| `[abc]` | exactly one character from `a`, `b` or `c` |
| other characters | literal |

The `*` operator means zero or more arbitrary characters, not just one.

Therefore:

```text
A*E
```

matches:

```text
AE
AbE
AbcdE
AbcdxyzE
```

The matcher operates on the complete input command.

The first matching rule in the configuration file wins.

## Configuration

Example:

```text
AT=OK\r\n
ATE0=OK\r\n
ATE1=OK\r\n
ATI=MyModem 1.0\r\nOK\r\n
AT+COPS?=+COPS: 0,0,"TestOperator"\r\nOK\r\n
AT+CPIN?=+CPIN: READY\r\nOK\r\n
```

The parser splits each rule at the first `=` so that `=` is allowed in the
answer.

Supported answer escapes:

- `\r` -> carriage return
- `\n` -> line feed
- `\t` -> tab
- `\\` -> backslash

Lines beginning with `#` are comments.

## Build

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
```

## Run

```bash
./build/atmodem \
    --device /dev/ttyUSB0 \
    --config config/modem.conf \
    --baud 115200
```

Default baud rate is 115200.

The serial interface is configured as:

- 8 data bits
- no parity
- 1 stop bit
- no hardware flow control
- raw mode

## Testing without hardware

A pseudo-terminal pair can be created with `socat`:

```bash
socat -d -d pty,raw,echo=0 pty,raw,echo=0
```

It will print something similar to:

```text
N PTY is /dev/pts/5
N PTY is /dev/pts/6
```

Run the server on one endpoint:

```bash
./build/atmodem -d /dev/pts/5 -c config/modem.conf
```

Open the other endpoint with a terminal program, for example:

```bash
screen /dev/pts/6 115200
```

Then enter:

```text
AT
ATI
AT+COPS?
AT+CPIN?
ATE0
ATI
ATE1
ATI
```

Expected responses include:

```text
OK

MyModem 1.0
OK

+COPS: 0,0,"TestOperator"
OK

+CPIN: READY
OK
```

## Echo behavior

The emulator starts with echo enabled.

`ATE0` disables echo for subsequent commands.

`ATE1` enables echo again.

The command is still matched against the configured rules and receives its
configured response.

A small amount of backspace handling is included so the emulator is usable
from a normal serial terminal, without depending on `readline`.

## Architecture

```text
                 +----------------+
TTY /dev/ttyXXX |  SerialPort    |
        ------->|                |
        <-------|  termios/read  |
                 +-------+--------+
                         |
                         v
                 +---------------+
                 |    Modem      |
                 | command input |
                 | echo / state  |
                 +-------+-------+
                         |
                         v
                 +---------------+
                 |    Matcher    |
                 | . * [...]     |
                 +-------+-------+
                         |
                         v
                 +---------------+
                 | Config rules  |
                 | expect=answer |
                 +---------------+
```

## Design notes

### Why no regex library?

The assignment explicitly prohibits it. The required language is small enough
to implement directly.

The matcher uses memoized dynamic programming. This prevents the potentially
exponential behavior of naive recursive backtracking when several `*`
operators are present.

### Why `termios`?

The task asks for Linux and says the application should behave like a typical
modem. `termios` is the native POSIX interface for configuring serial TTY
devices.

### Why pseudo-terminals for the demo?

A PTY pair behaves sufficiently like a serial endpoint for testing the server
without requiring a physical modem or USB-UART adapter.

## Project layout

```text
at_modem/
├── CMakeLists.txt
├── README.md
├── config/
│   └── modem.conf
├── include/
│   ├── config.h
│   ├── matcher.h
│   ├── modem.h
│   └── serial_port.h
├── src/
│   ├── config.cpp
│   ├── main.cpp
│   ├── matcher.cpp
│   ├── modem.cpp
│   └── serial_port.cpp
└── tests/
    ├── test_config.cpp
    └── test_matcher.cpp
```

