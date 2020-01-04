      mov    r0, #1
      mov    r1, #0
loop: store @16, r1
      add    r1, r1, r0
      store  @5, r1
      jz     loop