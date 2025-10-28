#include <linux/bpf.h>
#include <bpf/bpf_helpers.h>


char LICENSE[] SEC("license") = "GPL";

SEC("kprobe/__x64_sys_clone")
int map_access_fail(struct pt_regs *ctx) {
    char buffer[8] = {1,2,3,4,5,6,7,8};
    
    __u64 x = bpf_get_prandom_u32() % 10;
    __u64 y = bpf_get_prandom_u32() % 10;

    if (x < 1 || x > 3) return 0;
    if (y < 2 || y > 7) return 0;

    __u64 res = y / x;
    bpf_printk("x = %llu, y = %llu, res = %llu", x, y, res);

    char value = buffer[res];
    
    bpf_printk("Value read: %d", value);
    return 0;
}