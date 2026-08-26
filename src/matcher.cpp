#include "matcher.h"

#include <cstddef>
#include <optional>
#include <string_view>
#include <vector>

namespace atmodem {
namespace {

struct ClassToken {
    bool valid{};
    bool matched{};
    std::size_t next{};
};

ClassToken match_class(std::string_view pattern,
                       std::size_t pos,
                       unsigned char ch)
{
    // pos points at '['.
    std::size_t i = pos + 1U;
    bool found = false;

    if (i >= pattern.size()) {
        return {false, false, pos};
    }

    // Optional '^' negation is intentionally not supported: the task only
    // requires "[...]" as a set of accepted characters.
    for (; i < pattern.size() && pattern[i] != ']'; ++i) {
        if (static_cast<unsigned char>(pattern[i]) == ch) {
            found = true;
        }
    }

    if (i >= pattern.size()) {
        return {false, false, pos};
    }

    return {true, found, i + 1U};
}

bool match_impl(std::string_view pattern, std::string_view text)
{
    const std::size_t rows = pattern.size() + 1U;
    const std::size_t cols = text.size() + 1U;

    // -1 = unknown, 0 = false, 1 = true.
    std::vector<signed char> memo(rows * cols, -1);

    const auto index = [cols](std::size_t p, std::size_t s) {
        return p * cols + s;
    };

    // Recursive depth is bounded by pattern + text length. This is fine for
    // command-sized AT strings and keeps the wildcard semantics obvious.
    const auto solve = [&](auto&& self, std::size_t p, std::size_t s) -> bool {
        signed char& cached = memo[index(p, s)];
        if (cached != -1) {
            return cached == 1;
        }

        bool result = false;

        if (p == pattern.size()) {
            result = (s == text.size());
        } else if (pattern[p] == '*') {
            // '*' = zero or more arbitrary characters.
            result = self(self, p + 1U, s) ||
                     (s < text.size() && self(self, p, s + 1U));
        } else if (s < text.size()) {
            if (pattern[p] == '.') {
                result = self(self, p + 1U, s + 1U);
            } else if (pattern[p] == '[') {
                const ClassToken token =
                    match_class(pattern, p, static_cast<unsigned char>(text[s]));
                result = token.valid && token.matched &&
                         self(self, token.next, s + 1U);
            } else {
                result = (pattern[p] == text[s]) &&
                         self(self, p + 1U, s + 1U);
            }
        }

        cached = result ? 1 : 0;
        return result;
    };

    return solve(solve, 0U, 0U);
}

} // namespace

bool Matcher::match(std::string_view pattern, std::string_view text)
{
    return match_impl(pattern, text);
}

} // namespace atmodem
