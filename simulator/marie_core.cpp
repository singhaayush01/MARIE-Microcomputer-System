// ============================================================
// CSIS 3740 – Project 3 (Advanced): MARIE Simulator (Bonus Version)
// Author: Aayush Kumar Singh
// Description:
//   CPU simulator for the MARIE architecture.
//   Implements instruction fetch-decode-execute cycle.
//   Added:
//   ✅ SKIPCOND control logic
//   ✅ CLEAR (extra instruction)
//   ✅ Trace table with register output
//   ✅ Instruction execution count summary (Bonus)
//   ✅ Memory dump (Bonus)
// ============================================================

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
#include <array>
#include <unordered_map>
#include <cstdint>
using namespace std;

// ============================================================
// MARIE CPU Simulation Class
// ============================================================
struct Marie {
    static constexpr uint16_t MEM_SIZE = 4096;

    // Registers
    int16_t AC = 0, MBR = 0;
    uint16_t PC = 0, IR = 0, MAR = 0;

    // Memory and statistics
    array<uint16_t, MEM_SIZE> MEM{};
    unordered_map<string, uint64_t> icount;
    bool trace_console = true;

    // Utility functions
    static uint16_t HI4(uint16_t w) { return (w >> 12) & 0xF; }
    static uint16_t LO12(uint16_t w) { return w & 0x0FFF; }
    static string HEX4(uint16_t x) { stringstream s; s << uppercase << hex << setw(4) << setfill('0') << x; return s.str(); }

    // ---------------------------
    // Load program into memory
    // ---------------------------
    bool loadProgram(istream& in) {
        string line;
        uint16_t addr = 0;
        while (getline(in, line)) {
            auto p = line.find_first_of(";#");
            if (p != string::npos) line = line.substr(0, p);
            line.erase(remove_if(line.begin(), line.end(), ::isspace), line.end());
            if (line.empty()) continue;
            if (line[0] == '@') { addr = stoul(line.substr(1), nullptr, 16) & 0x0FFF; continue; }
            uint16_t w = stoul(line, nullptr, 16) & 0xFFFFu;
            MEM[addr++] = w;
        }
        return true;
    }

    // ---------------------------
    // Display trace per cycle
    // ---------------------------
    void traceRow(uint64_t cyc) {
        cout << setw(5) << cyc << " | " << HEX4(PC) << ' ' << HEX4(IR)
             << ' ' << HEX4((uint16_t)AC) << ' ' << HEX4(MAR)
             << ' ' << HEX4((uint16_t)MBR) << "\n";
    }

    // ============================================================
    // Core of simulator — Fetch-Decode-Execute
    // ============================================================
    bool step(uint64_t cyc) {
        // FETCH
        MAR = PC;
        MBR = MEM[MAR];
        IR = MBR;
        PC = (PC + 1) & 0x0FFF;

        uint16_t op = HI4(IR);
        uint16_t adr = LO12(IR);
        bool cont = true;

        switch (op) {
            case 0x1: icount["LOAD"]++;  MAR = adr; MBR = MEM[MAR]; AC = MBR; break;
            case 0x2: icount["STORE"]++; MAR = adr; MEM[MAR] = AC; break;
            case 0x3: icount["ADD"]++;   MAR = adr; MBR = MEM[MAR]; AC += MBR; break;
            case 0x4: icount["SUBT"]++;  MAR = adr; MBR = MEM[MAR]; AC -= MBR; break;
            case 0x5: icount["INPUT"]++; cout << "Input: "; cin >> AC; break;
            case 0x6: icount["OUTPUT"]++; cout << "Output: " << AC << "\n"; break;
            case 0x7: icount["HALT"]++;  cont = false; break;
            case 0x9: icount["JUMP"]++;  PC = adr; break;

            // ---------------------------
            // ✅ NEW SECTION: SKIPCOND
            // ---------------------------
            case 0x8: {
                icount["SKIPCOND"]++;
                uint16_t cond = adr & 0x0E00;
                bool take = false;
                if (cond == 0x000) take = (AC < 0);
                else if (cond == 0x0400) take = (AC == 0);
                else if (cond == 0x0800) take = (AC > 0);
                if (take) PC = (PC + 1) & 0x0FFF;
                break;
            }

            // ---------------------------
            // ✅ NEW SECTION: CLEAR (extra instruction)
            // ---------------------------
            case 0xB:
                icount["CLEAR"]++;
                AC = 0;
                break;

            default:
                cerr << "Unsupported opcode 0x" << hex << uppercase << op << dec << "\n";
                return false;
        }

        traceRow(cyc);
        return cont;
    }

    // ============================================================
    // BONUS FEATURE 1: Instruction count summary
    // ============================================================
    void printInstructionSummary() {
        cout << "\nInstruction Execution Counts:\n";
        cout << "---------------------------------\n";
        for (auto &p : icount) {
            cout << setw(10) << left << p.first << " : " << p.second << "\n";
        }
    }

    // ============================================================
    // BONUS FEATURE 2: Memory dump (first 32 addresses)
    // ============================================================
    void printMemoryDump(int maxCells = 32) {
        cout << "\nMemory Dump (first " << maxCells << " addresses):\n";
        cout << "---------------------------------\n";
        for (int i = 0; i < maxCells; i++) {
            cout << HEX4(i) << ": " << HEX4(MEM[i]) << "\n";
        }
    }

    // ============================================================
    // Run full program
    // ============================================================
    void run() {
        uint64_t cyc = 1;
        cout << "Cycle | PC   IR   AC    MAR  MBR\n---------------------------------\n";
        while (step(cyc++)) { if (cyc > 1000000) break; }
        cout << "Program halted.\n";

        // Print bonus summaries
        printInstructionSummary();
        printMemoryDump(20); // show first 20 memory cells
    }
};

// ============================================================
// Entry point
// ============================================================
int main(int argc, char** argv) {
    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " program.txt\n";
        return 1;
    }
    ifstream fin(argv[1]);
    if (!fin) { cerr << "Cannot open " << argv[1] << "\n"; return 1; }

    Marie m;
    m.loadProgram(fin);
    m.run();
    return 0;
}
