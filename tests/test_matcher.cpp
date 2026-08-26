#include "matcher.h"

#include <cassert>
#include <iostream>

using atmodem::Matcher;

int main()
{
    // Exact matching.
    assert(Matcher::match("AT", "AT"));
    assert(!Matcher::match("AT", "ATI"));

    // '.' matches exactly one character.
    assert(Matcher::match("A.E", "ATE"));
    assert(Matcher::match("A.E", "A1E"));
    assert(!Matcher::match("A.E", "AE"));
    assert(!Matcher::match("A.E", "A12E"));

    // '*' matches zero or more arbitrary characters.
    assert(Matcher::match("A*E", "AE"));
    assert(Matcher::match("A*E", "AbE"));
    assert(Matcher::match("A*E", "AbcdE"));
    assert(Matcher::match("A*E", "AbcdxyzE"));
    assert(!Matcher::match("A*E", "Abcd"));

    // '*' can occur at the beginning or middle.
    assert(Matcher::match("*", ""));
    assert(Matcher::match("*", "anything"));
    assert(Matcher::match("*OK", "OK"));
    assert(Matcher::match("*OK", "123OK"));
    assert(Matcher::match("AT*?", "AT?"));
    assert(Matcher::match("AT*?", "AT+COPS?"));

    // Character classes.
    assert(Matcher::match("ATE[01]", "ATE0"));
    assert(Matcher::match("ATE[01]", "ATE1"));
    assert(!Matcher::match("ATE[01]", "ATE2"));
    assert(Matcher::match("AT[+?]", "AT+"));
    assert(Matcher::match("AT[+?]", "AT?"));
    assert(!Matcher::match("AT[+?]", "ATC"));

    // Combination of operators.
    assert(Matcher::match("AT+COPS[?=]*", "AT+COPS?"));
    assert(Matcher::match("AT+COPS[?=]*", "AT+COPS=0"));
    assert(Matcher::match("AT+COPS[?=]*", "AT+COPS=0,0"));

    // Malformed character class does not match.
    assert(!Matcher::match("AT[01", "AT0"));

    // Complete-string matching.
    assert(!Matcher::match("AT", "xxAT"));
    assert(!Matcher::match("AT", "ATxx"));

    std::cout << "matcher tests: OK\n";
    return 0;
}
