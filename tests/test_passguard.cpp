// Minimal dependency-free tests. Run with: make test
#include <cctype>
#include <iostream>
#include <stdexcept>
#include <string>

#include "../src/checker.hpp"
#include "../src/generator.hpp"

static int failures = 0;
#define CHECK(cond)                                                          \
    do {                                                                     \
        if (!(cond)) {                                                       \
            std::cerr << "FAILED line " << __LINE__ << ": " #cond << "\n";   \
            ++failures;                                                      \
        }                                                                    \
    } while (0)

static bool anyOf(const std::string& s, int (*pred)(int)) {
    for (unsigned char c : s) if (pred(c)) return true;
    return false;
}

static bool tipContains(const passguard::CheckResult& r, const std::string& needle) {
    for (const auto& t : r.tips) if (t.find(needle) != std::string::npos) return true;
    return false;
}

int main() {
    using namespace passguard;

    // Checker
    CHECK(checkPassword("password").score == 0);
    CHECK(checkPassword("PASSWORD").score == 0);  // case-insensitive match
    CHECK(checkPassword("").score == 0);
    CHECK(checkPassword("tR7#kLq9!vZp2@Xm").score >= 3);
    CHECK(tipContains(checkPassword("abcd1234Zz!"), "sequences"));
    CHECK(tipContains(checkPassword("aaaBcd9!xyz1"), "repeating"));
    CHECK(checkPassword("Zebra!Quartz#91", {"zebra!quartz#91"}).score == 0);  // custom wordlist

    // Generator
    CHECK(generatePassword({20, true, true, true, false}).size() == 20);
    for (int i = 0; i < 200; ++i) {
        std::string pw = generatePassword({8, true, true, true, false});
        CHECK(anyOf(pw, std::islower));
        CHECK(anyOf(pw, std::isupper));
        CHECK(anyOf(pw, std::isdigit));
        CHECK(anyOf(pw, std::ispunct));
    }
    for (int i = 0; i < 100; ++i) {
        std::string pw = generatePassword({40, true, true, true, true});
        CHECK(pw.find_first_of("Il1O0o") == std::string::npos);
    }
    CHECK(!anyOf(generatePassword({30, true, true, false, false}), std::ispunct));

    bool threw = false;
    try { generatePassword({2, true, true, true, false}); } catch (const std::invalid_argument&) { threw = true; }
    CHECK(threw);

    if (failures == 0) std::cout << "All tests passed.\n";
    return failures == 0 ? 0 : 1;
}
