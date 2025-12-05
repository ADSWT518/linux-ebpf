#include <linux/bpf.h>
#include <bpf/bpf_helpers.h>

char LICENSE[] SEC("license") = "GPL";

// 使用 volatile 来防止编译器优化掉对 zero 变量的访问
// 这会强制编译器在运行时从内存中读取它的值
volatile const __u64 zero = 0;

SEC("kprobe/__x64_sys_clone")
int map_access_fail(struct pt_regs *ctx) {
    __u64 x = 2;
    // 编译器看到的是一个变量 y，无法在编译时确定其值
    // 因此必须生成一条真正的除法指令
    __u64 y = zero; 

    __u64 res = x / y;
    bpf_printk("x = %llu, y = %llu, res = %llu", x, y, res);

    return 0;
}