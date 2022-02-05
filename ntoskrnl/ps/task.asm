
global pop_regs

pop_regs:        ; you may also see this as context switching, so keep that in mind
    mov rsp, rdi ; point the stack register (rsp) to our own stack (rdi)
    pop r15      ; that works because rdi is the first function parameter, thus, our own stack
    pop r14      ; with pop, we restore each value of the stack to its very own register
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8

    pop rbp
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax
    
    add rsp, 16 ; move the stack pointer, skip two 64 bits values (see regs_t struct)
    
    ; the stack will now have what we want in order to switch to the new task, that is
    ; uint64_t rip;
    ; uint64_t cs;
    ; uint64_t rflags;
    ; uint64_t rsp;
    ; uint64_t ss;
    ; the iretq instruction will take care of using these values
    iretq