loop: 
    mov r0, #18
    mov r1, #0
    mov r2, #170
    mov r3, #0
    mov r4, #52
    store r3:r4, r2
    load r1, r3:r4
    nop
    nop
    nop
    mov    r5, #1
    add    r5, #1
    jnz    loop
