#include <cstdlib>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "checker.hpp"
#include "generator.hpp"

#ifdef _WIN32
#include <windows.h>
#else
#include <termios.h>
#include <unistd.h>
#endif

namespace {

// Reads a line without echoing it, so the password is not shown on screen
// and never ends up in shell history (unlike a command line argument).
std::string readHidden(const std::string& prompt) {
    std::cout << prompt << std::flush;
    std::string line;
#ifdef _WIN32
    HANDLE h = GetStdHandle(STD_INPUT_HANDLE);
    DWORD mode = 0;
    GetConsoleMode(h, &mode);
    SetConsoleMode(h, mode & ~ENABLE_ECHO_INPUT);
    std::getline(std::cin, line);
    SetConsoleMode(h, mode);
#else
    termios oldt{};
    bool tty = isatty(STDIN_FILENO) && tcgetattr(STDIN_FILENO, &oldt) == 0;
    if (tty) {
        termios newt = oldt;
        newt.c_lflag &= ~static_cast<tcflag_t>(ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    }
    std::getline(std::cin, line);
    if (tty) tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
#endif
    std::cout << "\n";
    return line;
}

std::vector<std::string> loadWordlist(const std::string& path) {
    std::vector<std::string> words;
    std::ifstream in(path);
    if (!in) throw std::runtime_error("cannot open wordlist: " + path);
    std::string w;
    while (std::getline(in, w)) {
        for (auto& c : w) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        if (!w.empty()) words.push_back(w);
    }
    return words;
}

void usage() {
    std::cout <<
        "passguard - password security toolkit\n\n"
        "Usage:\n"
        "  passguard check [--wordlist FILE]\n"
        "  passguard generate [-l LENGTH] [-n COUNT] [--no-upper] [--no-digits]\n"
        "                     [--no-symbols] [--avoid-ambiguous]\n";
}

int runCheck(const std::vector<std::string>& args) {
    std::vector<std::string> extra;
    for (std::size_t i = 0; i < args.size(); ++i) {
        if (args[i] == "--wordlist" && i + 1 < args.size()) extra = loadWordlist(args[++i]);
        else { usage(); return 2; }
    }
    auto r = passguard::checkPassword(readHidden("Password (hidden): "), extra);
    std::cout << "Strength: " << r.label << " (" << r.score << "/4)\n"
              << "Estimated entropy: " << r.entropyBits << " bits\n";
    for (const auto& tip : r.tips) std::cout << "  - " << tip << "\n";
    return 0;
}

int runGenerate(const std::vector<std::string>& args) {
    passguard::GeneratorOptions opts;
    int count = 1;
    for (std::size_t i = 0; i < args.size(); ++i) {
        const std::string& a = args[i];
        if ((a == "-l" || a == "--length") && i + 1 < args.size()) opts.length = std::stoi(args[++i]);
        else if ((a == "-n" || a == "--count") && i + 1 < args.size()) count = std::stoi(args[++i]);
        else if (a == "--no-upper") opts.upper = false;
        else if (a == "--no-digits") opts.digits = false;
        else if (a == "--no-symbols") opts.symbols = false;
        else if (a == "--avoid-ambiguous") opts.avoidAmbiguous = true;
        else { usage(); return 2; }
    }
    for (int i = 0; i < count; ++i) std::cout << passguard::generatePassword(opts) << "\n";
    return 0;
}

}  // namespace

int main(int argc, char* argv[]) {
    if (argc < 2) { usage(); return 2; }
    const std::string cmd = argv[1];
    std::vector<std::string> rest(argv + 2, argv + argc);
    try {
        if (cmd == "check") return runCheck(rest);
        if (cmd == "generate") return runGenerate(rest);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 2;
    }
    usage();
    return 2;
}
