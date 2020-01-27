      mov    r0, #0
      mov    r1, #0
loop: store  @255, r1
      store  @254, r1
      store  @253, r1
      store  @252, r1
wait: add    r0, 1
      jnz    wait
      add    r1, 1
      mov    r2, 1
      add    r2, 1
      jnz    loop