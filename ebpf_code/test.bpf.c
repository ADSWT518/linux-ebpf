#include <linux/bpf.h>
#include <bpf/bpf_helpers.h>

// 定义许可证，这是加载eBPF程序的强制要求
char LICENSE[] SEC("license") = "GPL";

// SEC宏用于指定程序应该被加载到哪个部分，以及它的类型和附加点
// 这里我们将程序附加到 clone 系统调用的内核实现上
SEC("kprobe/__x64_sys_clone")
int hello_world(struct pt_regs *ctx) {
    // 使用bpf_printk辅助函数打印一条消息到内核跟踪管道
    // bpf_printk是一个简单的调试工具，不建议在生产环境大量使用
    bpf_printk("Hello, World! a process was cloned.\n");
    return 0;
}