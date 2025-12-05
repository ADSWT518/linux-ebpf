// SPDX-License-Identifier: GPL-2.0
// File: simple_call_noinline.c
#include <linux/bpf.h>
#include <bpf/bpf_helpers.h>

const volatile int my_global_const = 5;

static long my_internal_func(long a, long b)
{
    long result = a + b;
    return result;
}

static long my_internal_func1(long a, long b)
{
    long result = a + b;
    result += my_global_const;
    return result;
}

SEC("kprobe/__x64_sys_execve")
int tracepoint_handler(struct pt_regs *ctx)
{
    long val1 = 10;
    long val2 = 20;
    long sum, sum1;

    sum = my_internal_func(val1, val2);
    sum1 = my_internal_func1(val1, val2);
    bpf_printk("Internal func result is %ld, %ld\n", sum, sum1);

    return 0;
}

char LICENSE[] SEC("license") = "GPL";