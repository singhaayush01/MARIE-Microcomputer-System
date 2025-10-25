; loop.asm — increment N times, store result to SUM
; SUM starts at 0, ONE = 1, N set via DEC
        LOAD SUM
LOOP:   ADD ONE
        STORE SUM
        LOAD N
        SUBT ONE
        STORE N
        LOAD N
        SKIPCOND ZE     ; skip next if N == 0
        JUMP LOOP
        OUTPUT
        HALT
SUM:    DEC 0
N:      DEC 5
ONE:    DEC 1
