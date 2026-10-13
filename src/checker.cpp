#include "checker.hpp"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstddef>
#include <set>

namespace passguard {
namespace {

const std::set<std::string> kCommon = {
    "password", "123456", "12345678", "123456789", "qwerty", "abc123",
    "111111", "iloveyou", "admin", "welcome", "letmein", "monkey",
    "dragon", "football", "password1", "passw0rd", "qwerty123", "1q2w3e4r"};

const char* kLabels[] = {"Very weak", "Weak", "Fair", "Strong", "Very strong"};

const char* kSequences[] = {"abcdefghijklmnopqrstuvwxyz", "0123456789",
                            "qwertyuiop", "asdfghjkl", "zxcvbnm"};

std::string toLower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return s;
}

bool isSymbol(unsigned char c) { return std::ispunct(c) != 0; }

bool hasSequence(const std::string& password, std::size_t len = 4) {
    const std::string lowered = toLower(password);
    for (const char* seq : kSequences) {
        const std::string s(seq);
        for (std::size_t i = 0; i + len <= s.size(); ++i) {
            std::string chunk = s.substr(i, len);
            std::string reversed(chunk.rbegin(), chunk.rend());
            if (lowered.find(chunk) != std::string::npos ||
                lowered.find(reversed) != std::string::npos) {
                return true;
            }
        }
    }
    return false;
}

bool hasRepeats(const std::string& password, std::size_t len = 3) {
    if (password.size() < len) return false;
    for (std::size_t i = 0; i + len <= password.size(); ++i) {
        bool same = true;
        for (std::size_t j = 1; j < len; ++j) {
            if (password[i + j] != password[i]) { same = false; break; }
        }
        if (same) return true;
    }
    return false;
}

}  // namespace

CheckResult checkPassword(const std::string& password,
                          const std::vector<std::string>& extraCommon) {
    CheckResult r{0, kLabels[0], 0.0, {}};

    if (password.empty()) {
        r.tips.push_back("Password is empty.");
        return r;
    }

    bool lower = false, upper = false, digit = false, symbol = false;
    for (unsigned char c : password) {
        if (std::islower(c)) lower = true;
        else if (std::isupper(c)) upper = true;
        else if (std::isdigit(c)) digit = true;
        else if (isSymbol(c)) symbol = true;
    }

    int pool = 0;
    if (lower) pool += 26;
    if (upper) pool += 26;
    if (digit) pool += 10;
    if (symbol) pool += 32;
    if (pool == 0) pool = 32;  // only non-ASCII or control characters
    double entropy = static_cast<double>(password.size()) * std::log2(pool);

    const std::string lowered = toLower(password);
    bool isCommon = kCommon.count(lowered) > 0 ||
                    std::find(extraCommon.begin(), extraCommon.end(), lowered) != extraCommon.end();
    if (isCommon) {
        r.entropyBits = std::round(entropy * 10) / 10;
        r.tips.push_back("This is a very common password.");
        return r;
    }

    if (password.size() < 12) r.tips.push_back("Use at least 12 characters.");
    if (!lower || !upper) r.tips.push_back("Mix uppercase and lowercase letters.");
    if (!digit) r.tips.push_back("Add some digits.");
    if (!symbol) r.tips.push_back("Add some symbols.");
    if (hasSequence(password)) {
        entropy *= 0.75;
        r.tips.push_back("Avoid sequences like 'abcd', '1234' or 'qwer'.");
    }
    if (hasRepeats(password)) {
        entropy *= 0.75;
        r.tips.push_back("Avoid repeating the same character 3+ times.");
    }

    if (entropy < 28) r.score = 0;
    else if (entropy < 36) r.score = 1;
    else if (entropy < 60) r.score = 2;
    else if (entropy < 80) r.score = 3;
    else r.score = 4;

    if (r.score >= 3 && r.tips.empty()) {
        r.tips.push_back("Looks good. Still, use a unique password for every site.");
    }

    r.label = kLabels[r.score];
    r.entropyBits = std::round(entropy * 10) / 10;
    return r;
}

}  // namespace passguard
