#pragma once

#include <string_view>

namespace atmodem {

class Matcher {
public:
    // Supported syntax:
    //   .       exactly one arbitrary character
    //   *       zero or more arbitrary characters
    //   [...]   exactly one character from the character class
    // All other characters are matched literally.
    static bool match(std::string_view pattern, std::string_view text);
};

} // namespace atmodem
