        mov r0, #1
        mov r1, #3
        sub r1, r1, r0
        jz end
        sub r1, r1, r0
        jz end
        sub r1, r1, r0
        jz end
        mov r1, #127
end:    nop
