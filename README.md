# CSIS 3740 – Project 3 (Advanced): MARIE Micro-Computer System

**Solo project (1 week).** Build a small MARIE system consisting of:
1) a CPU simulator, and 2) a minimal assembler converting symbolic MARIE assembly to hex machine code.

## Structure
- `simulator/marie_core.cpp` – CPU simulator starter
- `assembler/assembler_starter.cpp` – 2-pass assembler skeleton (labels + DEC)
- `examples/loop.asm` – example assembly
- `examples/skipcond_test.asm` – condition test
- `examples/loop.txt` – expected machine code (for reference)
- `docs/Project3_Handout_Advanced.pdf/.docx` – the handout

## Build (Ubuntu/VirtualBox)
```bash
g++ -std=c++17 -O2 simulator/marie_core.cpp -o marie
g++ -std=c++17 -O2 assembler/assembler_starter.cpp -o marie_asm
```

## Workflow
```bash
./marie_asm examples/loop.asm > examples/loop.txt
./marie examples/loop.txt --trace --save-trace trace.txt
```

## Phases (grading)
- Phase A – Control Logic (20): implement SKIPCOND + choose ONE extra instruction (CLEAR/ADDI/JUMPI)
- Phase B – Assembler (40): finish SKIPCOND encoding; support DEC; labels
- Phase C – Trace & Stats (30): console trace + save to file; instruction counts
- Documentation (10): README, comments, example I/O
- Bonus (10): memory dump, more instructions, better assembler
