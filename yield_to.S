    .text
    .global __co_yield_to
    .type __co_yield_to, @function
    // __yield_to(prev, next
    // eax edx ecx
    // ebx edi esi ebp
__co_yield_to:
    pushl %ebp
    movl %esp, %ebp

    // ebp    0 <=  ebp
    // ret_ip 4
    // prev   8
    // next   12
    pushl %ebx
    pushl %edi
    pushl %esi

    // save %esp
    movl 8(%ebp), %edi
    movl %esp, 0(%edi)
    // return prev
    movl %edi, %eax

    // set new esp
    movl 12(%ebp), %edi
    movl 0(%edi), %esp
    popl %esi
    popl %edi
    popl %ebx
    popl %ebp
    ret
    .size __co_yield_to, .-__co_yield_to

    .global __co_start
    .type __co_start, @function
__co_start:
    pushl %eax
    call __co_main@PLT
    .size __co_start, .-__co_start

