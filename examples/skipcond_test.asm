; skipcond_test.asm — demonstrate LT/ZE/GT
        LOAD POS
        SKIPCOND GT
        OUTPUT       ; skipped
        LOAD ZERO
        OUTPUT       ; prints zero
        HALT
NEG:    DEC -1
ZERO:   DEC 0
POS:    DEC 2
