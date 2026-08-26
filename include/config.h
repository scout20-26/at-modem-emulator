#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace atmodem {

struct CommandRule {
    std::string expect;
    std::string answer;
};

class Config {
public:
    static Config load(const std::string& path);

    [[nodiscard]] const std::vector<CommandRule>& rules() const noexcept {
        return rules_;
    }

private:
    std::vector<CommandRule> rules_;
};

std::string decode_escapes(std::string_view value);

} // namespace atmodem
