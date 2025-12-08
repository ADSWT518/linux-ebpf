#include <linux/bpf.h>
#include <limits.h>
#include <bpf/bpf_helpers.h>
#include "../../linux/tools/testing/selftests/bpf/progs/bpf_misc.h"

SEC("flow_dissector")
void flow_keys_illegal_variable_offset_alu(struct __sk_buff *skb)
{
	asm volatile("					\
	r6 = r1;					\
	r7 = *(u64*)(r6 + %[flow_keys_off]);		\
	call %[bpf_get_prandom_u32];			\
	r8 = r0;					\
	r8 &= 8;					\
	r7 += r8;					\
	r0 = *(u64*)(r7 + 0);				\
	// exit;						\
"	:
	: __imm_const(flow_keys_off, offsetof(struct __sk_buff, flow_keys)),
	  __imm(bpf_get_prandom_u32)
	: __clobber_all);
}

char LICENSE[] SEC("license") = "GPL";
