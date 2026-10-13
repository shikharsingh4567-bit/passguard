#pragma once
#include <string>

namespace passguard {

struct GeneratorOptions {
    int length = 16;
    bool upper = true;
    bool digits = true;
    bool symbols = true;
    bool avoidAmbiguous = false;  // skip look-alike characters: I l 1 O 0 o
};

// Generates a random password with at least one character of each enabled class.
// Throws std::invalid_argument if the options are not valid.
std::string generatePassword(const GeneratorOptions& opts = {});

}  // namespace passguard
