    mov    r1, #0
    mov    r2, #64
    mov    r3, #128
    mov    r4, #192
    mov    r11, #255
    mov    r12, #254
    mov    r13, #253
    mov    r14, #252
    mov    r15, #255
repeat:
    ; Send new values to the PWM drivers
    store  r15:r11, r1
    store  r15:r12, r2
    store  r14:r13, r3
    store  r14:r14, r4
    mov    r5, #0  ; Init outer loop
outerwait:    
    mov    r6, #0  ; Init inner loop
wait:    
    add    r6, #2
    jnz    wait
    add    r5, #1
    jnz    outerwait
    ; Update PWM values
    add    r1, #2
    add    r2, #1
    add    r3, #3
    add    r4, #5
    ; Loop infinitely
    mov    r5, #1
    add    r5, #1
    jnz    repeat 
