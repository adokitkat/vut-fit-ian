# Projekt 2

## Autor: Adam Múdry (xmudry01)

### Dôvod pádu z logu

```text
./crash

...

      KERNEL: vmlinux
    DUMPFILE: vmcore  [PARTIAL DUMP]
        CPUS: 4
        DATE: Wed Apr 17 01:40:54 2019
      UPTIME: 00:25:24
LOAD AVERAGE: 0.12, 0.06, 0.05
       TASKS: 121
    NODENAME: cda-class-leader
     RELEASE: 4.15.3-300.fc27.x86_64
     VERSION: #1 SMP Tue Feb 13 17:02:01 UTC 2018
     MACHINE: x86_64  (2996 Mhz)
      MEMORY: 4 GB
       PANIC: "Kernel panic - not syncing: softlockup: hung tasks"
         PID: 61
     COMMAND: "kworker/1:1"
        TASK: ffff8f6f76863e80  [THREAD_INFO: ffff8f6f76863e80]
         CPU: 1
       STATE: TASK_RUNNING (PANIC)

crash> log | grep BUG
[ 1552.035262] watchdog: BUG: soft lockup - CPU#1 stuck for 22s! [kworker/1:1:61]
```

CPU 1 vyvolalo kernel panic lebo zistilo soft lockup.

### Backtrace panikujúceho CPU

```text
crash> bt
PID: 61     TASK: ffff8f6f76863e80  CPU: 1   COMMAND: "kworker/1:1"
 #0 [ffff8f6f7b243d68] machine_kexec at ffffffff8c0582ae
 #1 [ffff8f6f7b243dc0] __crash_kexec at ffffffff8c13d211
 #2 [ffff8f6f7b243e80] panic at ffffffff8c0a7d42
 #3 [ffff8f6f7b243f08] watchdog_timer_fn at ffffffff8c1653be
 #4 [ffff8f6f7b243f38] __hrtimer_run_queues at ffffffff8c11f2b8
 #5 [ffff8f6f7b243f88] hrtimer_interrupt at ffffffff8c11f946
 #6 [ffff8f6f7b243fd8] smp_apic_timer_interrupt at ffffffff8ca03002
 #7 [ffff8f6f7b243ff0] apic_timer_interrupt at ffffffff8ca02182
--- <IRQ stack> ---
 #8 [ffffb39780993ce8] apic_timer_interrupt at ffffffff8ca02182
    [exception RIP: smp_call_function_many+0x227]
    RIP: ffffffff8c134057  RSP: ffffb39780993d90  RFLAGS: 00000202
    RAX: 0000000000000003  RBX: ffff8f6f7b261cc0  RCX: 0000000000000003
    RDX: ffff8f6f7b2e69c0  RSI: 0000000000000000  RDI: ffff8f6f7ac359f8
    RBP: 0000000000000010   R8: fffffffffffffff8   R9: 000000000000000d
    R10: ffffe05584e84b00  R11: 0000000000000f20  R12: 0000000000000001
    R13: 0000000000021c80  R14: ffffffff8c02c700  R15: 0000000000000000
    ORIG_RAX: ffffffffffffff11  CS: 0010  SS: 0018
 #9 [ffffb39780993dc8] on_each_cpu at ffffffff8c1340d8
#10 [ffffb39780993de8] text_poke_bp at ffffffff8c02d578
#11 [ffffb39780993e20] __jump_label_transform at ffffffff8c02a453
#12 [ffffb39780993e40] arch_jump_label_transform at ffffffff8c02a48b
#13 [ffffb39780993e58] __jump_label_update at ffffffff8c1ce743
#14 [ffffb39780993e78] static_key_enable_cpuslocked at ffffffff8c1cea52
#15 [ffffb39780993e88] static_key_enable at ffffffff8c1cea96
#16 [ffffb39780993e98] process_one_work at ffffffff8c0c0a77
#17 [ffffb39780993ed8] worker_thread at ffffffff8c0c0cce
#18 [ffffb39780993f10] kthread at ffffffff8c0c7a63
#19 [ffffb39780993f50] ret_from_fork at ffffffff8ca001b5
```

Na CPU1 bola volaná funkcia `smp_call_function_many` ktorá vie spustiť v 2. argumente zadanú funkciu na ostatných CPU jadrách.
Keďže ale zrejme niektoré z ostatných jadier nereagovalo, tak sa v tejto funkcii čakalo tak dlho, že kernerl vyvolal soft lockup panic.

### Disassembly poslednej vykonávanej funkcie do bodu pádu (na CPU1)

```text
crash> dis -l smp_call_function_many
/usr/src/debug/kernel-4.15.fc27/linux-4.15.3-300.fc27.x86_64/kernel/smp.c: 405
0xffffffff8c133e30 <smp_call_function_many>:    nopl   0x0(%rax,%rax,1) [FTRACE NOP]
0xffffffff8c133e35 <smp_call_function_many+0x5>:        push   %r15
0xffffffff8c133e37 <smp_call_function_many+0x7>:        push   %r14
0xffffffff8c133e39 <smp_call_function_many+0x9>:        mov    %rdx,%r15
0xffffffff8c133e3c <smp_call_function_many+0xc>:        push   %r13
0xffffffff8c133e3e <smp_call_function_many+0xe>:        push   %r12
0xffffffff8c133e40 <smp_call_function_many+0x10>:       mov    %rsi,%r14
0xffffffff8c133e43 <smp_call_function_many+0x13>:       push   %rbp
0xffffffff8c133e44 <smp_call_function_many+0x14>:       push   %rbx
0xffffffff8c133e45 <smp_call_function_many+0x15>:       mov    %rdi,%rbp
0xffffffff8c133e48 <smp_call_function_many+0x18>:       mov    %ecx,%r12d
0xffffffff8c133e4b <smp_call_function_many+0x1b>:       sub    $0x8,%rsp
0xffffffff8c133e4f <smp_call_function_many+0x1f>:       mov    %cl,0x3(%rsp)
/usr/src/debug/kernel-4.15.fc27/linux-4.15.3-300.fc27.x86_64/kernel/smp.c: 407
0xffffffff8c133e53 <smp_call_function_many+0x23>:       mov    %gs:0x73edb2d6(%rip),%ebx        # 0xf130
/usr/src/debug/kernel-4.15.fc27/linux-4.15.3-300.fc27.x86_64/./include/linux/cpumask.h: 330
0xffffffff8c133e5a <smp_call_function_many+0x2a>:       mov    %ebx,%r13d
/usr/src/debug/kernel-4.15.fc27/linux-4.15.3-300.fc27.x86_64/./arch/x86/include/asm/bitops.h: 332
0xffffffff8c133e5d <smp_call_function_many+0x2d>:       bt     %r13,0x124733b(%rip)        # 0xffffffff8d37b1a0
/usr/src/debug/kernel-4.15.fc27/linux-4.15.3-300.fc27.x86_64/kernel/smp.c: 415
0xffffffff8c133e65 <smp_call_function_many+0x35>:       jae    0xffffffff8c133e73 <smp_call_function_many+0x43>
/usr/src/debug/kernel-4.15.fc27/linux-4.15.3-300.fc27.x86_64/./arch/x86/include/asm/paravirt.h: 772
0xffffffff8c133e67 <smp_call_function_many+0x37>:       pushfq
0xffffffff8c133e68 <smp_call_function_many+0x38>:       pop    %rax
0xffffffff8c133e69 <smp_call_function_many+0x39>:       nopl   0x0(%rax,%rax,1)
/usr/src/debug/kernel-4.15.fc27/linux-4.15.3-300.fc27.x86_64/kernel/smp.c: 415
0xffffffff8c133e6e <smp_call_function_many+0x3e>:       test   $0x2,%ah
0xffffffff8c133e71 <smp_call_function_many+0x41>:       je     0xffffffff8c133eaa <smp_call_function_many+0x7a>
/usr/src/debug/kernel-4.15.fc27/linux-4.15.3-300.fc27.x86_64/kernel/smp.c: 419
0xffffffff8c133e73 <smp_call_function_many+0x43>:       mov    $0xffffffff8d37b1a0,%rdx
0xffffffff8c133e7a <smp_call_function_many+0x4a>:       mov    %rbp,%rsi
0xffffffff8c133e7d <smp_call_function_many+0x4d>:       mov    $0xffffffff,%edi
0xffffffff8c133e82 <smp_call_function_many+0x52>:       callq  0xffffffff8c87f2b0 <cpumask_next_and>
/usr/src/debug/kernel-4.15.fc27/linux-4.15.3-300.fc27.x86_64/kernel/smp.c: 420
0xffffffff8c133e87 <smp_call_function_many+0x57>:       cmp    %eax,%ebx
/usr/src/debug/kernel-4.15.fc27/linux-4.15.3-300.fc27.x86_64/kernel/smp.c: 419
0xffffffff8c133e89 <smp_call_function_many+0x59>:       mov    %eax,%r8d
/usr/src/debug/kernel-4.15.fc27/linux-4.15.3-300.fc27.x86_64/kernel/smp.c: 420
0xffffffff8c133e8c <smp_call_function_many+0x5c>:       je     0xffffffff8c133fce <smp_call_function_many+0x19e>
/usr/src/debug/kernel-4.15.fc27/linux-4.15.3-300.fc27.x86_64/kernel/smp.c: 424
0xffffffff8c133e92 <smp_call_function_many+0x62>:       cmp    0x124760b(%rip),%r8d        # 0xffffffff8d37b4a4
0xffffffff8c133e99 <smp_call_function_many+0x69>:       jb     0xffffffff8c133ec1 <smp_call_function_many+0x91>
/usr/src/debug/kernel-4.15.fc27/linux-4.15.3-300.fc27.x86_64/kernel/smp.c: 471
0xffffffff8c133e9b <smp_call_function_many+0x6b>:       add    $0x8,%rsp
0xffffffff8c133e9f <smp_call_function_many+0x6f>:       pop    %rbx
0xffffffff8c133ea0 <smp_call_function_many+0x70>:       pop    %rbp
0xffffffff8c133ea1 <smp_call_function_many+0x71>:       pop    %r12
0xffffffff8c133ea3 <smp_call_function_many+0x73>:       pop    %r13
0xffffffff8c133ea5 <smp_call_function_many+0x75>:       pop    %r14
0xffffffff8c133ea7 <smp_call_function_many+0x77>:       pop    %r15
0xffffffff8c133ea9 <smp_call_function_many+0x79>:       retq
/usr/src/debug/kernel-4.15.fc27/linux-4.15.3-300.fc27.x86_64/kernel/smp.c: 415
0xffffffff8c133eaa <smp_call_function_many+0x7a>:       mov    0x1825310(%rip),%eax        # 0xffffffff8d9591c0
0xffffffff8c133eb0 <smp_call_function_many+0x80>:       test   %eax,%eax
0xffffffff8c133eb2 <smp_call_function_many+0x82>:       jne    0xffffffff8c133e73 <smp_call_function_many+0x43>
0xffffffff8c133eb4 <smp_call_function_many+0x84>:       cmpb   $0x0,0x124214d(%rip)        # 0xffffffff8d376008
0xffffffff8c133ebb <smp_call_function_many+0x8b>:       jne    0xffffffff8c133e73 <smp_call_function_many+0x43>
0xffffffff8c133ebd <smp_call_function_many+0x8d>:       (bad)
0xffffffff8c133ebf <smp_call_function_many+0x8f>:       jmp    0xffffffff8c133e73 <smp_call_function_many+0x43>
/usr/src/debug/kernel-4.15.fc27/linux-4.15.3-300.fc27.x86_64/kernel/smp.c: 428
0xffffffff8c133ec1 <smp_call_function_many+0x91>:       mov    %r8d,%edi
0xffffffff8c133ec4 <smp_call_function_many+0x94>:       mov    $0xffffffff8d37b1a0,%rdx
0xffffffff8c133ecb <smp_call_function_many+0x9b>:       mov    %rbp,%rsi
0xffffffff8c133ece <smp_call_function_many+0x9e>:       mov    %r8d,0x4(%rsp)
0xffffffff8c133ed3 <smp_call_function_many+0xa3>:       callq  0xffffffff8c87f2b0 <cpumask_next_and>
/usr/src/debug/kernel-4.15.fc27/linux-4.15.3-300.fc27.x86_64/kernel/smp.c: 429
0xffffffff8c133ed8 <smp_call_function_many+0xa8>:       cmp    %eax,%ebx
0xffffffff8c133eda <smp_call_function_many+0xaa>:       mov    0x4(%rsp),%r8d
0xffffffff8c133edf <smp_call_function_many+0xaf>:       je     0xffffffff8c13405e <smp_call_function_many+0x22e>
/usr/src/debug/kernel-4.15.fc27/linux-4.15.3-300.fc27.x86_64/kernel/smp.c: 433
0xffffffff8c133ee5 <smp_call_function_many+0xb5>:       mov    0x12475b9(%rip),%ecx        # 0xffffffff8d37b4a4
0xffffffff8c133eeb <smp_call_function_many+0xbb>:       cmp    %ecx,%eax
0xffffffff8c133eed <smp_call_function_many+0xbd>:       jae    0xffffffff8c133fe7 <smp_call_function_many+0x1b7>
/usr/src/debug/kernel-4.15.fc27/linux-4.15.3-300.fc27.x86_64/kernel/smp.c: 438
0xffffffff8c133ef3 <smp_call_function_many+0xc3>:       mov    $0x21cc0,%rbx
0xffffffff8c133efa <smp_call_function_many+0xca>:       add    %gs:0x73edb236(%rip),%rbx        # 0xf138
/usr/src/debug/kernel-4.15.fc27/linux-4.15.3-300.fc27.x86_64/./include/linux/bitmap.h: 236
0xffffffff8c133f02 <smp_call_function_many+0xd2>:       mov    0x8(%rbx),%rdi
0xffffffff8c133f06 <smp_call_function_many+0xd6>:       mov    $0xffffffff8d37b1a0,%rdx
0xffffffff8c133f0d <smp_call_function_many+0xdd>:       mov    %rbp,%rsi
0xffffffff8c133f10 <smp_call_function_many+0xe0>:       callq  0xffffffff8c433ac0 <__bitmap_and>
/usr/src/debug/kernel-4.15.fc27/linux-4.15.3-300.fc27.x86_64/./include/linux/cpumask.h: 318
0xffffffff8c133f15 <smp_call_function_many+0xe5>:       mov    0x8(%rbx),%rax
/usr/src/debug/kernel-4.15.fc27/linux-4.15.3-300.fc27.x86_64/./arch/x86/include/asm/bitops.h: 140
0xffffffff8c133f19 <smp_call_function_many+0xe9>:       btr    %r13,(%rax)
/usr/src/debug/kernel-4.15.fc27/linux-4.15.3-300.fc27.x86_64/./include/linux/bitmap.h: 322
0xffffffff8c133f1d <smp_call_function_many+0xed>:       mov    0x1247581(%rip),%esi        # 0xffffffff8d37b4a4
0xffffffff8c133f23 <smp_call_function_many+0xf3>:       mov    0x8(%rbx),%rdi
0xffffffff8c133f27 <smp_call_function_many+0xf7>:       callq  0xffffffff8c4340f0 <__bitmap_weight>
/usr/src/debug/kernel-4.15.fc27/linux-4.15.3-300.fc27.x86_64/kernel/smp.c: 444
0xffffffff8c133f2c <smp_call_function_many+0xfc>:       test   %eax,%eax
0xffffffff8c133f2e <smp_call_function_many+0xfe>:       je     0xffffffff8c133e9b <smp_call_function_many+0x6b>
/usr/src/debug/kernel-4.15.fc27/linux-4.15.3-300.fc27.x86_64/./include/linux/bitmap.h: 205
0xffffffff8c133f34 <smp_call_function_many+0x104>:      mov    0x124756a(%rip),%edx        # 0xffffffff8d37b4a4
/usr/src/debug/kernel-4.15.fc27/linux-4.15.3-300.fc27.x86_64/./include/linux/string.h: 329
0xffffffff8c133f3a <smp_call_function_many+0x10a>:      mov    0x10(%rbx),%rdi
0xffffffff8c133f3e <smp_call_function_many+0x10e>:      xor    %esi,%esi
0xffffffff8c133f40 <smp_call_function_many+0x110>:      mov    $0xffffffff,%ebp
/usr/src/debug/kernel-4.15.fc27/linux-4.15.3-300.fc27.x86_64/kernel/smp.c: 456
0xffffffff8c133f45 <smp_call_function_many+0x115>:      mov    $0x21c80,%r13
/usr/src/debug/kernel-4.15.fc27/linux-4.15.3-300.fc27.x86_64/./include/linux/bitmap.h: 205
0xffffffff8c133f4c <smp_call_function_many+0x11c>:      add    $0x3f,%rdx
0xffffffff8c133f50 <smp_call_function_many+0x120>:      shr    $0x6,%rdx
/usr/src/debug/kernel-4.15.fc27/linux-4.15.3-300.fc27.x86_64/./include/linux/bitmap.h: 206
0xffffffff8c133f54 <smp_call_function_many+0x124>:      shl    $0x3,%rdx
/usr/src/debug/kernel-4.15.fc27/linux-4.15.3-300.fc27.x86_64/./include/linux/string.h: 329
0xffffffff8c133f58 <smp_call_function_many+0x128>:      callq  0xffffffff8c891a40 <__memset>
/usr/src/debug/kernel-4.15.fc27/linux-4.15.3-300.fc27.x86_64/kernel/smp.c: 448
0xffffffff8c133f5d <smp_call_function_many+0x12d>:      mov    0x8(%rbx),%rsi
0xffffffff8c133f61 <smp_call_function_many+0x131>:      mov    %ebp,%edi
0xffffffff8c133f63 <smp_call_function_many+0x133>:      callq  0xffffffff8c87f290 <cpumask_next>
0xffffffff8c133f68 <smp_call_function_many+0x138>:      cmp    0x1247536(%rip),%eax        # 0xffffffff8d37b4a4
0xffffffff8c133f6e <smp_call_function_many+0x13e>:      mov    %eax,%ebp
0xffffffff8c133f70 <smp_call_function_many+0x140>:      jae    0xffffffff8c134007 <smp_call_function_many+0x1d7>
/usr/src/debug/kernel-4.15.fc27/linux-4.15.3-300.fc27.x86_64/kernel/smp.c: 449
0xffffffff8c133f76 <smp_call_function_many+0x146>:      movslq %ebp,%rcx
0xffffffff8c133f79 <smp_call_function_many+0x149>:      mov    (%rbx),%rdi
0xffffffff8c133f7c <smp_call_function_many+0x14c>:      add    -0x72eb2c00(,%rcx,8),%rdi
/usr/src/debug/kernel-4.15.fc27/linux-4.15.3-300.fc27.x86_64/./include/linux/compiler.h: 183
0xffffffff8c133f84 <smp_call_function_many+0x154>:      mov    0x18(%rdi),%eax
/usr/src/debug/kernel-4.15.fc27/linux-4.15.3-300.fc27.x86_64/kernel/smp.c: 108
0xffffffff8c133f87 <smp_call_function_many+0x157>:      test   $0x1,%al
0xffffffff8c133f89 <smp_call_function_many+0x159>:      je     0xffffffff8c133f94 <smp_call_function_many+0x164>
/usr/src/debug/kernel-4.15.fc27/linux-4.15.3-300.fc27.x86_64/./arch/x86/include/asm/processor.h: 649
0xffffffff8c133f8b <smp_call_function_many+0x15b>:      pause
/usr/src/debug/kernel-4.15.fc27/linux-4.15.3-300.fc27.x86_64/./include/linux/compiler.h: 183
0xffffffff8c133f8d <smp_call_function_many+0x15d>:      mov    0x18(%rdi),%eax
/usr/src/debug/kernel-4.15.fc27/linux-4.15.3-300.fc27.x86_64/kernel/smp.c: 108
0xffffffff8c133f90 <smp_call_function_many+0x160>:      test   $0x1,%al
0xffffffff8c133f92 <smp_call_function_many+0x162>:      jne    0xffffffff8c133f8b <smp_call_function_many+0x15b>
/usr/src/debug/kernel-4.15.fc27/linux-4.15.3-300.fc27.x86_64/kernel/smp.c: 114
0xffffffff8c133f94 <smp_call_function_many+0x164>:      orl    $0x1,0x18(%rdi)
/usr/src/debug/kernel-4.15.fc27/linux-4.15.3-300.fc27.x86_64/kernel/smp.c: 452
0xffffffff8c133f98 <smp_call_function_many+0x168>:      cmpb   $0x0,0x3(%rsp)
0xffffffff8c133f9d <smp_call_function_many+0x16d>:      je     0xffffffff8c133fa3 <smp_call_function_many+0x173>
/usr/src/debug/kernel-4.15.fc27/linux-4.15.3-300.fc27.x86_64/kernel/smp.c: 453
0xffffffff8c133f9f <smp_call_function_many+0x16f>:      orl    $0x2,0x18(%rdi)
/usr/src/debug/kernel-4.15.fc27/linux-4.15.3-300.fc27.x86_64/kernel/smp.c: 454
0xffffffff8c133fa3 <smp_call_function_many+0x173>:      mov    %r14,0x8(%rdi)
/usr/src/debug/kernel-4.15.fc27/linux-4.15.3-300.fc27.x86_64/kernel/smp.c: 455
0xffffffff8c133fa7 <smp_call_function_many+0x177>:      mov    %r15,0x10(%rdi)
/usr/src/debug/kernel-4.15.fc27/linux-4.15.3-300.fc27.x86_64/kernel/smp.c: 456
0xffffffff8c133fab <smp_call_function_many+0x17b>:      mov    %r13,%rdx
0xffffffff8c133fae <smp_call_function_many+0x17e>:      add    -0x72eb2c00(,%rcx,8),%rdx
/usr/src/debug/kernel-4.15.fc27/linux-4.15.3-300.fc27.x86_64/./include/linux/llist.h: 221
0xffffffff8c133fb6 <smp_call_function_many+0x186>:      mov    %rdi,%rsi
0xffffffff8c133fb9 <smp_call_function_many+0x189>:      callq  0xffffffff8c43a950 <llist_add_batch>
/usr/src/debug/kernel-4.15.fc27/linux-4.15.3-300.fc27.x86_64/kernel/smp.c: 456
0xffffffff8c133fbe <smp_call_function_many+0x18e>:      test   %al,%al
0xffffffff8c133fc0 <smp_call_function_many+0x190>:      je     0xffffffff8c133f5d <smp_call_function_many+0x12d>
/usr/src/debug/kernel-4.15.fc27/linux-4.15.3-300.fc27.x86_64/./include/linux/cpumask.h: 302
0xffffffff8c133fc2 <smp_call_function_many+0x192>:      mov    0x10(%rbx),%rax
0xffffffff8c133fc6 <smp_call_function_many+0x196>:      mov    %ebp,%edx
/usr/src/debug/kernel-4.15.fc27/linux-4.15.3-300.fc27.x86_64/./arch/x86/include/asm/bitops.h: 97
0xffffffff8c133fc8 <smp_call_function_many+0x198>:      bts    %rdx,(%rax)
0xffffffff8c133fcc <smp_call_function_many+0x19c>:      jmp    0xffffffff8c133f5d <smp_call_function_many+0x12d>
/usr/src/debug/kernel-4.15.fc27/linux-4.15.3-300.fc27.x86_64/kernel/smp.c: 421
0xffffffff8c133fce <smp_call_function_many+0x19e>:      mov    $0xffffffff8d37b1a0,%rdx
0xffffffff8c133fd5 <smp_call_function_many+0x1a5>:      mov    %rbp,%rsi
0xffffffff8c133fd8 <smp_call_function_many+0x1a8>:      mov    %ebx,%edi
0xffffffff8c133fda <smp_call_function_many+0x1aa>:      callq  0xffffffff8c87f2b0 <cpumask_next_and>
0xffffffff8c133fdf <smp_call_function_many+0x1af>:      mov    %eax,%r8d
0xffffffff8c133fe2 <smp_call_function_many+0x1b2>:      jmpq   0xffffffff8c133e92 <smp_call_function_many+0x62>
/usr/src/debug/kernel-4.15.fc27/linux-4.15.3-300.fc27.x86_64/kernel/smp.c: 471
0xffffffff8c133fe7 <smp_call_function_many+0x1b7>:      add    $0x8,%rsp
/usr/src/debug/kernel-4.15.fc27/linux-4.15.3-300.fc27.x86_64/kernel/smp.c: 434
0xffffffff8c133feb <smp_call_function_many+0x1bb>:      movzbl %r12b,%ecx
0xffffffff8c133fef <smp_call_function_many+0x1bf>:      mov    %r15,%rdx
/usr/src/debug/kernel-4.15.fc27/linux-4.15.3-300.fc27.x86_64/kernel/smp.c: 471
0xffffffff8c133ff2 <smp_call_function_many+0x1c2>:      pop    %rbx
/usr/src/debug/kernel-4.15.fc27/linux-4.15.3-300.fc27.x86_64/kernel/smp.c: 434
0xffffffff8c133ff3 <smp_call_function_many+0x1c3>:      mov    %r14,%rsi
0xffffffff8c133ff6 <smp_call_function_many+0x1c6>:      mov    %r8d,%edi
/usr/src/debug/kernel-4.15.fc27/linux-4.15.3-300.fc27.x86_64/kernel/smp.c: 471
0xffffffff8c133ff9 <smp_call_function_many+0x1c9>:      pop    %rbp
0xffffffff8c133ffa <smp_call_function_many+0x1ca>:      pop    %r12
0xffffffff8c133ffc <smp_call_function_many+0x1cc>:      pop    %r13
0xffffffff8c133ffe <smp_call_function_many+0x1ce>:      pop    %r14
0xffffffff8c134000 <smp_call_function_many+0x1d0>:      pop    %r15
/usr/src/debug/kernel-4.15.fc27/linux-4.15.3-300.fc27.x86_64/kernel/smp.c: 434
0xffffffff8c134002 <smp_call_function_many+0x1d2>:      jmpq   0xffffffff8c133ab0 <smp_call_function_single>
/usr/src/debug/kernel-4.15.fc27/linux-4.15.3-300.fc27.x86_64/./arch/x86/include/asm/smp.h: 126
0xffffffff8c134007 <smp_call_function_many+0x1d7>:      mov    0x10f1da2(%rip),%rax        # 0xffffffff8d225db0
0xffffffff8c13400e <smp_call_function_many+0x1de>:      mov    0x10(%rbx),%rdi
0xffffffff8c134012 <smp_call_function_many+0x1e2>:      callq  0xffffffff8cc03000 <__indirect_thunk_start>
/usr/src/debug/kernel-4.15.fc27/linux-4.15.3-300.fc27.x86_64/kernel/smp.c: 463
0xffffffff8c134017 <smp_call_function_many+0x1e7>:      test   %r12b,%r12b
/usr/src/debug/kernel-4.15.fc27/linux-4.15.3-300.fc27.x86_64/kernel/smp.c: 464
0xffffffff8c13401a <smp_call_function_many+0x1ea>:      mov    $0xffffffff,%eax
/usr/src/debug/kernel-4.15.fc27/linux-4.15.3-300.fc27.x86_64/kernel/smp.c: 463
0xffffffff8c13401f <smp_call_function_many+0x1ef>:      je     0xffffffff8c133e9b <smp_call_function_many+0x6b>
/usr/src/debug/kernel-4.15.fc27/linux-4.15.3-300.fc27.x86_64/kernel/smp.c: 464
0xffffffff8c134025 <smp_call_function_many+0x1f5>:      mov    0x8(%rbx),%rsi
0xffffffff8c134029 <smp_call_function_many+0x1f9>:      mov    %eax,%edi
0xffffffff8c13402b <smp_call_function_many+0x1fb>:      callq  0xffffffff8c87f290 <cpumask_next>                   // (2)
0xffffffff8c134030 <smp_call_function_many+0x200>:      cmp    0x124746e(%rip),%eax        # 0xffffffff8d37b4a4
0xffffffff8c134036 <smp_call_function_many+0x206>:      jae    0xffffffff8c133e9b <smp_call_function_many+0x6b>
/usr/src/debug/kernel-4.15.fc27/linux-4.15.3-300.fc27.x86_64/kernel/smp.c: 467
0xffffffff8c13403c <smp_call_function_many+0x20c>:      movslq %eax,%rcx
0xffffffff8c13403f <smp_call_function_many+0x20f>:      mov    (%rbx),%rdx
0xffffffff8c134042 <smp_call_function_many+0x212>:      add    -0x72eb2c00(,%rcx,8),%rdx
/usr/src/debug/kernel-4.15.fc27/linux-4.15.3-300.fc27.x86_64/./include/linux/compiler.h: 183
0xffffffff8c13404a <smp_call_function_many+0x21a>:      mov    0x18(%rdx),%ecx
/usr/src/debug/kernel-4.15.fc27/linux-4.15.3-300.fc27.x86_64/kernel/smp.c: 108
0xffffffff8c13404d <smp_call_function_many+0x21d>:      and    $0x1,%ecx
0xffffffff8c134050 <smp_call_function_many+0x220>:      je     0xffffffff8c13405c <smp_call_function_many+0x22c>
/usr/src/debug/kernel-4.15.fc27/linux-4.15.3-300.fc27.x86_64/./arch/x86/include/asm/processor.h: 649
0xffffffff8c134052 <smp_call_function_many+0x222>:      pause
/usr/src/debug/kernel-4.15.fc27/linux-4.15.3-300.fc27.x86_64/./include/linux/compiler.h: 183
0xffffffff8c134054 <smp_call_function_many+0x224>:      mov    0x18(%rdx),%ecx
/usr/src/debug/kernel-4.15.fc27/linux-4.15.3-300.fc27.x86_64/kernel/smp.c: 108
0xffffffff8c134057 <smp_call_function_many+0x227>:      and    $0x1,%ecx                                           // (1)
0xffffffff8c13405a <smp_call_function_many+0x22a>:      jne    0xffffffff8c134052 <smp_call_function_many+0x222>
0xffffffff8c13405c <smp_call_function_many+0x22c>:      jmp    0xffffffff8c134025 <smp_call_function_many+0x1f5>
/usr/src/debug/kernel-4.15.fc27/linux-4.15.3-300.fc27.x86_64/kernel/smp.c: 430
0xffffffff8c13405e <smp_call_function_many+0x22e>:      mov    $0xffffffff8d37b1a0,%rdx
0xffffffff8c134065 <smp_call_function_many+0x235>:      mov    %rbp,%rsi
0xffffffff8c134068 <smp_call_function_many+0x238>:      mov    %eax,%edi
0xffffffff8c13406a <smp_call_function_many+0x23a>:      callq  0xffffffff8c87f2b0 <cpumask_next_and>
0xffffffff8c13406f <smp_call_function_many+0x23f>:      mov    0x4(%rsp),%r8d
0xffffffff8c134074 <smp_call_function_many+0x244>:      jmpq   0xffffffff8c133ee5 <smp_call_function_many+0xb5>
0xffffffff8c134079 <smp_call_function_many+0x249>:      nopl   0x0(%rax)
```

Z backtrace viem, že zaseknutie nastalo na nariadku `0xffffffff8c134057 <smp_call_function_many+0x227>` (1). O pár riadkov vyššie je `0xffffffff8c13402b <smp_call_function_many+0x1fb>: callq 0xffffffff8c87f290 <cpumask_next>` (2), kde sa volá funckia `cpumask_next` a viem, že vracia hodnotu do registru `EAX` a jeho obsah sa už do zaseknutia nemenil, takže sa v ňom stále nachádza návratová hodnota - z backtrace viem taktiež to, že `RAX: 0000000000000003` a to je teda index CPU jadra (tj. `CPU3`), na ktoré sa čaká.

Keď z backtrace viem, na ktorej inštrukcii (1) sme zastavili, tak viem aj vyčítať, že task vlastne viazne na týchto 4 inštrukciách:

```text
...
0xffffffff8c134052 <smp_call_function_many+0x222>:      pause
0xffffffff8c134054 <smp_call_function_many+0x224>:      mov    0x18(%rdx),%ecx                                     // (3)
0xffffffff8c134057 <smp_call_function_many+0x227>:      and    $0x1,%ecx                                           // (1)
0xffffffff8c13405a <smp_call_function_many+0x22a>:      jne    0xffffffff8c134052 <smp_call_function_many+0x222>
...
```

Inštukcia `jne` dookola skáče na `<smp_call_function_many+0x222>` adresu, lebo v `0xffffffff8c134057 <smp_call_function_many+0x227>: and $0x1,%ecx` (1) sa neustále nastavuje zero flag `ZF = 0`, pričom sa tam deje `and` inštrukcia medzi číslom 1 a `ECX` registrom.

Hodnota v `ECX` registri je do neho nahrávaná na riadku `0xffffffff8c134054 <smp_call_function_many+0x224>: mov 0x18(%rdx),%ecx` (3) z `0x18(%rdx)` (`RDX+0x18`).

//, a teda to je aj adresa blokujúceho zámku. Z backrace viem, že hodnota v `ECX` a teda `RDX+0x18` je `RCX: 0000000000000003`.

### Backtrace CPU3

```text
crash> bt -a
...
PID: 2818   TASK: ffff8f6f76f0be80  CPU: 3   COMMAND: "yum"
 #0 [fffffe000008ce50] crash_nmi_callback at ffffffff8c04bd73
 #1 [fffffe000008ce58] nmi_handle at ffffffff8c028b53
 #2 [fffffe000008ceb0] default_do_nmi at ffffffff8c02901e
 #3 [fffffe000008ced0] do_nmi at ffffffff8c0291d2
 #4 [fffffe000008cef0] end_repeat_nmi at ffffffff8ca01bc8
    [exception RIP: queued_spin_lock_slowpath+0x21]
    RIP: ffffffff8c0f6741  RSP: ffffb39780aafc18  RFLAGS: 00000002
    RAX: 0000000000000001  RBX: 0000000000000010  RCX: 0000000000000002
    RDX: 0000000000000001  RSI: 0000000000000001  RDI: ffff8f6f7ffd2bc0
    RBP: ffffb39780aafd38   R8: 0000000000000000   R9: 00000000000a7d3b
    R10: 0000000000000000  R11: 0000000000000000  R12: 00000000014280ca
    R13: 0000000000000000  R14: ffff8f6f7ffd2680  R15: ffffb39780aafd38
    ORIG_RAX: ffffffffffffffff  CS: 0010  SS: 0000
--- <NMI exception stack> ---
 #5 [ffffb39780aafc18] queued_spin_lock_slowpath at ffffffff8c0f6741
 #6 [ffffb39780aafc18] get_page_from_freelist at ffffffff8c1de263
 #7 [ffffb39780aafd30] __alloc_pages_nodemask at ffffffff8c1e083e
 #8 [ffffb39780aafd90] alloc_pages_vma at ffffffff8c2420dc
 #9 [ffffb39780aafdd0] __handle_mm_fault at ffffffff8c2160ab
#10 [ffffb39780aafe80] handle_mm_fault at ffffffff8c216a4a
#11 [ffffb39780aafeb0] __do_page_fault at ffffffff8c0673bd
#12 [ffffb39780aaff20] do_page_fault at ffffffff8c067672
#13 [ffffb39780aaff50] async_page_fault at ffffffff8ca0183c
    RIP: 00007f241d83230b  RSP: 00007ffddc9a01f8  RFLAGS: 00010202
    RAX: 0000559df6b906b8  RBX: 00000000001916f8  RCX: 0000559df6b9167e
    RDX: 000000000000065e  RSI: 00007f240247a032  RDI: 0000559df6b90fc0
    RBP: 00007ffddc9a0430   R8: fffffffffffffff8   R9: 0000000000000fe6
    R10: 0000559df6b9169e  R11: 0000559df6b906b8  R12: 0000000000000000
    R13: 0000559df3c2bac0  R14: 00000000000ecac4  R15: 0000559df4dd9840
    ORIG_RAX: ffffffffffffffff  CS: 0033  SS: 002b
```

Vidím, že CPU3 sa zaseklo na `queued_spin_lock_slowpath+0x21`.

### Disassembly `queued_spin_lock_slowpath`

```text
crash> dis -l queued_spin_lock_slowpath
/usr/src/debug/kernel-4.15.fc27/linux-4.15.3-300.fc27.x86_64/kernel/locking/qspinlock.c: 295
0xffffffff8c0f6720 <queued_spin_lock_slowpath>: nopl   0x0(%rax,%rax,1) [FTRACE NOP]
0xffffffff8c0f6725 <queued_spin_lock_slowpath+0x5>:     nopl   0x0(%rax,%rax,1)
/usr/src/debug/kernel-4.15.fc27/linux-4.15.3-300.fc27.x86_64/./arch/x86/include/asm/atomic.h: 187
0xffffffff8c0f672a <queued_spin_lock_slowpath+0xa>:     mov    $0x1,%edx
/usr/src/debug/kernel-4.15.fc27/linux-4.15.3-300.fc27.x86_64/./include/linux/compiler.h: 183
0xffffffff8c0f672f <queued_spin_lock_slowpath+0xf>:     mov    (%rdi),%eax
/usr/src/debug/kernel-4.15.fc27/linux-4.15.3-300.fc27.x86_64/./arch/x86/include/asm/qspinlock.h: 68
0xffffffff8c0f6731 <queued_spin_lock_slowpath+0x11>:    test   %eax,%eax
0xffffffff8c0f6733 <queued_spin_lock_slowpath+0x13>:    jne    0xffffffff8c0f673f <queued_spin_lock_slowpath+0x1f> // (5)
/usr/src/debug/kernel-4.15.fc27/linux-4.15.3-300.fc27.x86_64/./arch/x86/include/asm/atomic.h: 187
0xffffffff8c0f6735 <queued_spin_lock_slowpath+0x15>:    lock cmpxchg %edx,(%rdi)                                   // (6)
/usr/src/debug/kernel-4.15.fc27/linux-4.15.3-300.fc27.x86_64/./arch/x86/include/asm/qspinlock.h: 70
0xffffffff8c0f6739 <queued_spin_lock_slowpath+0x19>:    test   %eax,%eax
0xffffffff8c0f673b <queued_spin_lock_slowpath+0x1b>:    jne    0xffffffff8c0f672f <queued_spin_lock_slowpath+0xf>
0xffffffff8c0f673d <queued_spin_lock_slowpath+0x1d>:    repz retq                                                  // (7)
/usr/src/debug/kernel-4.15.fc27/linux-4.15.3-300.fc27.x86_64/./arch/x86/include/asm/processor.h: 649
0xffffffff8c0f673f <queued_spin_lock_slowpath+0x1f>:    pause
0xffffffff8c0f6741 <queued_spin_lock_slowpath+0x21>:    jmp    0xffffffff8c0f672f <queued_spin_lock_slowpath+0xf>  // (4)
...
```

Vyzerá to tak, že aby sa program dostal na `0xffffffff8c0f6741 <queued_spin_lock_slowpath+0x21>` (4), tak musí skočiť z `0xffffffff8c0f6733 <queued_spin_lock_slowpath+0x13>` (5), čím preskakuje `queued_spin_lock_slowpath+0x15` (6) až `queued_spin_lock_slowpath+0x1d` (7) a teda aj return `repz retq`.

Po prezretí zdrojového kódu viem, že táto funkcia má ako svoj 1. argument pointer na `struct qspinlock` - viem že ten je predaný v registri `RDI`, ktorého hodnotu viem z backtrace CPU3 - `RDI: ffff8f6f7ffd2bc0`. To je teda adresa blokujúceho zámku.

Pozriem sa na adresu zámku cez `struct`:

```text
crash> struct qspinlock ffff8f6f7ffd2bc0
struct qspinlock {
  val = {
    counter = 0x1
  }
}
```

A vidím, že jeho hodnota je 1.

### Disassembly `get_page_from_freelist (ffffffff8c1de263)`

Z backtrace viem, že funkcia ktorá volá `queued_spin_lock_slowpath` je `get_page_from_freelist` na `ffffffff8c1de263`. Disassemly:

```text
crash> dis -l ffffffff8c1de263
/usr/src/debug/kernel-4.15.fc27/linux-4.15.3-300.fc27.x86_64/mm/page_alloc.c: 2355
0xffffffff8c1de263 <get_page_from_freelist+0x353>:      movl   $0x0,0x78(%rsp)
```

Nájdem si zdrojový kód:

```c
/*
 * Obtain a specified number of elements from the buddy allocator, all under
 * a single hold of the lock, for efficiency.  Add them to the supplied list.
 * Returns the number of new pages which were placed at *list.
 */
static int rmqueue_bulk(struct zone *zone, unsigned int order, unsigned long count, struct list_head *list, int migratetype)
...
```

A viem, ktorá C funkcia sa pokúšala získať zámok (pre alokáciu stránok) a nedarilo sa jej to...

### Záver

Na CPU1 sa vyhlásil kernel panic kvôli soft lockupu vo `smp_call_function_many`, kde sa však čakalo na CPU3 v `queued_spin_lock_slowpath`, čo zas čakalo na získavanie zámku.
