    .text
    .p2align 3
    .global __co_yield_to_asm
    .type __co_yield_to_asm, @function
    // __yield_to(prev, next
    // argument: stack, right to left
    // callee save: ebx edi esi ebp
    // eax edx ecx
    // return: eax, edx
    //=====================================
    // x86_64:
    // argument: rdi, rsi, rdx, rcx, r8, r9
    // callee save: rbx, rsp, rbp, r12, r13, r14, r15
    // return: rax, rdx
__co_yield_to_asm:
    pushq %rbp
    movq %rsp, %rbp

    pushq %rbx
    pushq %r12
    pushq %r13
    pushq %r14
    pushq %r15

    // save prev rsp
    movq %rsp, 0(%rdi)

    // switch stack to next
    movq 0(%rsi), %rsp

    // return prev
    movq %rdi, %rax

    popq %r15
    popq %r14
    popq %r13
    popq %r12
    popq %rbx
    popq %rbp
    ret
    .size __co_yield_to_asm, .-__co_yield_to_asm

    .p2align 3
    .global __co_start_asm
    .type __co_start_asm, @function
__co_start_asm:
    // co
    popq %rsi
    movq %rax, %rdi
    // __co_main(prev, self)
    call __co_main@PLT
    .size __co_start_asm, .-__co_start_asm
