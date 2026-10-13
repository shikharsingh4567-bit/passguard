#include "generator.hpp"

#include <algorithm>
#include <random>
#include <stdexcept>
#include <vector>

namespace passguard {
namespace {

std::string removeAmbiguous(const std::string& s) {
    std::string out;
    for (char c : s) {
        if (std::string("Il1O0o").find(c) == std::string::npos) out += c;
    }
    return out;
}

}  // namespace

std::string generatePassword(const GeneratorOptions& opts) {
    std::vector<std::string> classes = {"abcdefghijklmnopqrstuvwxyz"};
    if (opts.upper) classes.push_back("ABCDEFGHIJKLMNOPQRSTUVWXYZ");
    if (opts.digits) classes.push_back("0123456789");
    if (opts.symbols) classes.push_back("!@#$%^&*()-_=+[]{};:,.<>?/");

    if (opts.avoidAmbiguous) {
        for (auto& cls : classes) cls = removeAmbiguous(cls);
    }
    if (opts.length < static_cast<int>(classes.size())) {
        throw std::invalid_argument("length must be at least " + std::to_string(classes.size()) +
                                    " for the chosen options");
    }
    if (opts.length > 256) throw std::invalid_argument("length must be 256 or less");

    std::string alphabet;
    for (const auto& cls : classes) alphabet += cls;

    // std::random_device is non-deterministic on mainstream platforms and is
    // used directly (no seeded PRNG) so the output cannot be predicted.
    std::random_device rd;
    auto pick = [&rd](const std::string& s) {
        std::uniform_int_distribution<std::size_t> dist(0, s.size() - 1);
        return s[dist(rd)];
    };

    std::string pw;
    for (const auto& cls : classes) pw += pick(cls);  // one from each class
    while (static_cast<int>(pw.size()) < opts.length) pw += pick(alphabet);

    // Fisher-Yates shuffle so the guaranteed characters are not always first.
    for (std::size_t i = pw.size() - 1; i > 0; --i) {
        std::uniform_int_distribution<std::size_t> dist(0, i);
        std::swap(pw[i], pw[dist(rd)]);
    }
    return pw;
}

}  // namespace passguard
