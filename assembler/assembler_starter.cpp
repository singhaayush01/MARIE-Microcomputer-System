// ============================================================
// CSIS 3740 – Project 3 (Advanced): MARIE Assembler (Completed)
// Author: Aayush Kumar Singh
// Description:
//   Two-pass assembler that translates MARIE assembly language
//   into 16-bit hexadecimal machine code.
//
//   Added Features:
//   ✅ SKIPCOND encoding for LT/ZE/GT (Phase A & B)
//   ✅ CLEAR instruction (extra opcode 0xB)
//   ✅ Readability, comments, and structure improved
// ============================================================

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <cstdint>
using namespace std;

// ---------------------------
// Data structure for assembly lines
// ---------------------------
struct AsmLine {
    string label, op, arg, raw;
    int lineno;
};

// ---------------------------
// Utility: trim whitespace
// ---------------------------
static string trim(const string& s) {
    size_t i = 0, j = s.size();
    while (i < j && isspace((unsigned char)s[i])) i++;
    while (j > i && isspace((unsigned char)s[j - 1])) j--;
    return s.substr(i, j - i);
}

// ---------------------------
// Convert to uppercase (for consistency)
// ---------------------------
static string upper(string s) {
    for (char& c : s) c = (char)toupper((unsigned char)c);
    return s;
}

// ---------------------------
// Tokenizer: splits lines into label/op/arg
// ---------------------------
vector<AsmLine> tokenize(istream& in) {
    vector<AsmLine> v;
    string line;
    int ln = 0;
    while (getline(in, line)) {
        ln++;
        // Remove comments (# or ;)
        auto p = line.find_first_of(";#");
        if (p != string::npos) line = line.substr(0, p);
        line = trim(line);
        if (line.empty()) continue;

        AsmLine a;
        a.raw = line;
        a.lineno = ln;

        // Detect label
        if (line.find(':') != string::npos) {
            auto pos = line.find(':');
            a.label = trim(line.substr(0, pos));
            line = line.substr(pos + 1);
        }

        // Extract mnemonic and argument
        stringstream ss(line);
        ss >> a.op;
        string rest;
        getline(ss, rest);
        a.arg = trim(rest);
        v.push_back(a);
    }
    return v;
}

// ============================================================
//  Assembler Class — Two Pass Implementation
// ============================================================
struct Assembler {
    unordered_map<string, uint16_t> sym; // Symbol table
    vector<AsmLine> IR;                  // Intermediate representation
    uint16_t loc = 0;

    // ---------------------------
    // Pass 1: build symbol table
    // ---------------------------
    bool pass1() {
        loc = 0;
        for (auto& a : IR) {
            if (!a.label.empty()) {
                if (sym.count(a.label)) {
                    cerr << "Duplicate label: " << a.label << "\n";
                    return false;
                }
                sym[a.label] = loc;
            }
            loc += 1; // each instruction/data = 1 word
        }
        return true;
    }

    static uint16_t addr12(uint16_t x) { return x & 0x0FFF; }
    static uint16_t op4(uint16_t x) { return (x & 0xF) << 12; }

    // ---------------------------
    // Pass 2: generate machine code
    // ---------------------------
    bool pass2(ostream& out) {
        for (auto& a : IR) {
            uint16_t word = 0;
            string op = upper(a.op);
            string arg = upper(a.arg);

            // Handle label-only lines
            if (op.empty()) {
                word = 0x0000;
                out << setw(4) << setfill('0') << hex << uppercase << word << "\n";
                continue;
            }

            // Handle DEC constants
            if (op == "DEC") {
                long long val = 0;
                try {
                    val = stoll(arg, nullptr, 0);
                } catch (...) {
                    cerr << "Invalid DEC value at line " << a.lineno << "\n";
                }
                word = static_cast<uint16_t>(val);
                out << setw(4) << setfill('0') << hex << uppercase << word << "\n";
                continue;
            }

            // Address resolver
            auto resolve = [&](const string& s) -> uint16_t {
                if (s.empty()) return 0;
                if (sym.count(s)) return sym[s];
                try {
                    return static_cast<uint16_t>(stoul(s, nullptr, 0));
                } catch (...) {
                    cerr << "Cannot resolve address: '" << s << "' (line " << a.lineno << ")\n";
                    return 0;
                }
            };

            auto A = [&](uint16_t opc, const string& argS) { return op4(opc) | addr12(resolve(argS)); };

            // ---------------------------
            // OPCODE ENCODING
            // ---------------------------
            if (op == "LOAD") word = A(0x1, arg);
            else if (op == "STORE") word = A(0x2, arg);
            else if (op == "ADD") word = A(0x3, arg);
            else if (op == "SUBT") word = A(0x4, arg);
            else if (op == "INPUT") word = A(0x5, "0");
            else if (op == "OUTPUT") word = A(0x6, "0");
            else if (op == "HALT") word = A(0x7, "0");
            else if (op == "JUMP") word = A(0x9, arg);

            // ---------------------------
            // ✅ NEW SECTION: SKIPCOND encoding
            // Bits 11–9 represent condition:
            //   000 = AC < 0 (LT)
            //   400 = AC == 0 (ZE)
            //   800 = AC > 0 (GT)
            // ---------------------------
            else if (op == "SKIPCOND") {
                uint16_t cond = 0;
                if (arg == "LT" || arg == "0") cond = 0x000;
                else if (arg == "ZE" || arg == "400") cond = 0x400;
                else if (arg == "GT" || arg == "800") cond = 0x800;
                else cerr << "Unknown SKIPCOND arg '" << a.arg << "' at line " << a.lineno << "\n";
                word = op4(0x8) | (cond & 0x0E00);
            }

            // ---------------------------
            // ✅ NEW SECTION: CLEAR instruction (extra opcode 0xB)
            // Behavior: AC ← 0
            // ---------------------------
            else if (op == "CLEAR") {
                word = op4(0xB);
            }

            // Optional extensions (for bonus)
            else if (op == "ADDI") word = A(0xC, arg);
            else if (op == "JUMPI") word = A(0xD, arg);
            else cerr << "Unknown opcode '" << a.op << "' at line " << a.lineno << "\n";

            out << setw(4) << setfill('0') << hex << uppercase << word << "\n";
        }
        return true;
    }
};

// ---------------------------
// Main driver
// ---------------------------
int main(int argc, char** argv) {
    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " program.asm\n";
        return 1;
    }
    ifstream fin(argv[1]);
    if (!fin) {
        cerr << "Cannot open " << argv[1] << "\n";
        return 1;
    }

    auto toks = tokenize(fin);
    Assembler as;
    as.IR = std::move(toks);

    if (!as.pass1()) return 1;
    if (!as.pass2(cout)) return 1;
    return 0;
}
