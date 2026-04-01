// SPDX-License-Identifier: GPL-2.0
#include <linux/bpf.h>
#include <bpf/bpf_helpers.h>
static __attribute__((noinline)) void dummy_subprog(void)
{
	asm volatile("": : :"memory");
}

#define C10()  \
	dummy_subprog(); dummy_subprog(); dummy_subprog(); dummy_subprog(); dummy_subprog(); \
	dummy_subprog(); dummy_subprog(); dummy_subprog(); dummy_subprog(); dummy_subprog()

#define C100() \
	C10(); C10(); C10(); C10(); C10(); C10(); C10(); C10(); C10(); C10()

#define C1000() \
	C100(); C100(); C100(); C100(); C100(); C100(); C100(); C100(); C100(); C100()

SEC("tc")
int bench_bpf_call(struct __sk_buff *skb)
{
	C1000();
	return 0;
}

char LICENSE[] SEC("license") = "GPL";