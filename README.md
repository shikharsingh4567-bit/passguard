# passguard

A small command line password toolkit written in **C++17** with no external dependencies.

- **Check** how strong a password is, with an entropy estimate and tips to improve it
- **Generate** random passwords using a non-deterministic source (`std::random_device`)

## Why I built it

I wanted a project that goes beyond lab exercises: a real CLI with multiple source files, a header/implementation split, tests and a Makefile. It also taught me why passwords should never be passed as command line arguments (they end up in shell history), so `check` reads the password from a hidden prompt instead.

## Build and run

Requires `g++` (or any C++17 compiler) and `make`.

```bash
make              # builds ./passguard
make test         # builds and runs the tests
```

### Check a password

```text
$ ./passguard check
Password (hidden):
Strength: Very weak (0/4)
Estimated entropy: 46.5 bits
  - This is a very common password.
```

Add your own list of known-bad passwords (one per line):

```bash
./passguard check --wordlist my_list.txt
```

### Generate passwords

```bash
./passguard generate                       # one 16-character password
./passguard generate -l 24 -n 5            # five 24-character passwords
./passguard generate --no-symbols          # letters and digits only
./passguard generate --avoid-ambiguous     # skip look-alikes: I l 1 O 0 o
```

Every generated password contains at least one character from each enabled class.

## How it works

| File | Purpose |
|------|---------|
| `src/checker.cpp` | Estimates entropy as `length × log2(pool size)`, lowers it for sequences (`abcd`, `1234`, `qwer`) and repeats (`aaa`), and rejects common passwords |
| `src/generator.cpp` | Picks one character per class, fills the rest from the full alphabet, then shuffles (Fisher-Yates) |
| `src/main.cpp` | Argument parsing and hidden password input |
| `tests/test_passguard.cpp` | Dependency-free tests (`make test`) |

## Limitations

- The strength score is a **heuristic estimate**, not a guarantee. It only knows a small built-in list of common passwords, so use `--wordlist` for better coverage.
- Hidden input on Windows uses the Windows console API; I have only tested the Linux build so far.
- It does not check whether a password has appeared in a data breach.

## Ideas for next versions

- Passphrase generator (random words)
- Breach check using the Have I Been Pwned k-anonymity API
- Pattern detection for dates and keyboard walks
