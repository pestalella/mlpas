loop: mov r0, #18
    mov r1, #0
    mov r2, #170
    store @52, r2
    load r1, @52
    nop
    nop
    nop
    mov    r5, #1
    add    r5, #1
    jnz    loop
