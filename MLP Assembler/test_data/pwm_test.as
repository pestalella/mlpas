    mov    r14, #0
    mov    r2, #64
    mov    r3, #128
    mov    r4, #192
    ; Send new values to the PWM drivers
loop:
    store  @255, r14
    store  @254, r2
    store  @253, r3
    store  @252, r4
    mov    r5, #0  ; Init outer loop
outerwait:
    mov    r6, #0  ; Init inner loop
wait:
    add    r6, #2
    jnz    wait
    add    r5, #1
    jnz    outerwait
    ; Update PWM values
    add    r14, #2
    add    r2, #1
    add    r3, #3
    add    r4, #5
    ; Loop infinitely
    mov    r5, #1
    add    r5, #1
    jnz    loop 
