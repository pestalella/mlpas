      mov    r0, #16
      mov    r1, #32
loop: store  @16, r0
      add    r0, #1
      store  @5, r0
      sub    r2, r1, r0
      jnz    loop