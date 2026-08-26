#include "config.h"

#include <cctype>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace atmodem {
namespace {

std::string trim_left(std::string_view value)
{
    std::size_t pos = 0U;
    while (pos < value.size() &&
           std::isspace(static_cast<unsigned char>(value[pos])) != 0) {
        ++pos;
    }
    return std::string(value.substr(pos));
}

} // namespace

std::string decode_escapes(std::string_view value)
{
    std::string result;
    result.reserve(value.size());

    for (std::size_t i = 0U; i < value.size(); ++i) {
        if (value[i] != '\\' || i + 1U >= value.size()) {
            result += value[i];
            continue;
        }

        ++i;
        switch (value[i]) {
        case 'r':
            result += '\r';
            break;
        case 'n':
            result += '\n';
            break;
        case 't':
            result += '\t';
            break;
        case '\\':
            result += '\\';
            break;
        default:
            // Unknown escape: keep the escaped character literally.
            result += value[i];
            break;
        }
    }

    return result;
}

Config Config::load(const std::string& path)
{
    std::ifstream input(path);
    if (!input) {
        throw std::runtime_error("cannot open config file: " + path);
    }

    Config config;
    std::string line;
    std::size_t line_number = 0U;

    while (std::getline(input, line)) {
        ++line_number;

        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }

        const std::string left_trimmed = trim_left(line);
        if (left_trimmed.empty() || left_trimmed.front() == '#') {
            continue;
        }

        const std::size_t separator = line.find('=');
        if (separator == std::string::npos) {
            std::ostringstream message;
            message << "invalid config line " << line_number
                    << ": expected expect=answer";
            throw std::runtime_error(message.str());
        }

        std::string expect = line.substr(0U, separator);
        std::string answer = line.substr(separator + 1U);

        // A leading UTF-8 BOM is tolerated on the first rule.
        if (config.rules_.empty() &&
            expect.size() >= 3U &&
            static_cast<unsigned char>(expect[0]) == 0xEFU &&
            static_cast<unsigned char>(expect[1]) == 0xBBU &&
            static_cast<unsigned char>(expect[2]) == 0xBFU) {
            expect.erase(0U, 3U);
        }

        config.rules_.push_back(
            CommandRule{std::move(expect), decode_escapes(answer)});
    }

    if (config.rules_.empty()) {
        throw std::runtime_error("config file contains no rules");
    }

    return config;
}

} // namespace atmodem
