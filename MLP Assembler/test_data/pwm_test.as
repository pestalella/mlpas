           mov    r0, #0
           mov    r1, #0
           mov    r2, #64
           mov    r3, #128
           mov    r4, #192
loop:      store  @255, r1
           store  @254, r2
           store  @253, r3
           store  @252, r4
      
           mov    r5, #1
outerwait: mov    r6, #1
wait:      add    r6, #1
           jnz    wait
           add    r5, #1
           jnz    outerwait

           add    r1, #1
           add    r2, #1
           add    r3, #1
           add    r4, #1
           mov    r5, #1
           add    r5, #1
           jnz    loop