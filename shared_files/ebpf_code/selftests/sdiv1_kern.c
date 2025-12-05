#include <linux/bpf.h>
#include <limits.h>
#include <bpf/bpf_helpers.h>
#include "../../../linux/tools/testing/selftests/bpf/progs/bpf_misc.h"

SEC("kprobe/__x64_sys_execve")
void sdiv32_overflow_rr(struct pt_regs *ctx)
{
	asm volatile ("					\
	w2 = %[int_min];				\
	w3 = -1;					\
	w4 = w2;					\
	w2 s/= w3;					\
	r0 = 0;						\
	if w2 != w4 goto +1;				\
	r0 = 1;						\
	// exit;						\
"	:
	: __imm_const(int_min, INT_MIN)
	: __clobber_all);
}

char LICENSE[] SEC("license") = "GPL";
