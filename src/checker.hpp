#pragma once
#include <string>
#include <vector>

namespace passguard {

struct CheckResult {
    int score;                       // 0 (very weak) to 4 (very strong)
    std::string label;               // human readable label for the score
    double entropyBits;              // rough entropy estimate in bits
    std::vector<std::string> tips;   // suggestions to improve the password
};

// Rates a password with simple heuristics (length, variety, patterns).
// `extraCommon` is an optional list of known-weak passwords (lowercase).
CheckResult checkPassword(const std::string& password,
                          const std::vector<std::string>& extraCommon = {});

}  // namespace passguard
