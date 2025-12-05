// tail_call_kern.c
#include <linux/bpf.h>
#include <bpf/bpf_helpers.h>

// 定义一个结构体来存储进程信息
struct process_info {
    __u32 pid;
    char comm[16];
};

struct {
    __uint(type, BPF_MAP_TYPE_PROG_ARRAY);
    __uint(max_entries, 10);
    __uint(key_size, sizeof(__u32));
    __uint(value_size, sizeof(__u32));
} prog_array SEC(".maps");

SEC("tracepoint")
int bpf_prog2(void *ctx) {
    struct process_info info = {};
    __u64 pid_tgid = bpf_get_current_pid_tgid();

    info.pid = pid_tgid >> 32; // 高32位是 PID
    bpf_get_current_comm(&info.comm, sizeof(info.comm));

    bpf_printk("Program 2 (Tail Call): Triggered by PID %u (%s)\n", info.pid, info.comm);
    return 0;
}

SEC("tp/syscalls/sys_enter_newuname")
int bpf_prog1(void *ctx) {
    struct process_info info = {};
    __u64 pid_tgid = bpf_get_current_pid_tgid();

    info.pid = pid_tgid >> 32; // 高32位是 PID
    bpf_get_current_comm(&info.comm, sizeof(info.comm));
    
    bpf_printk("Program 1: Triggered by PID %u (%s), preparing tail call...\n", info.pid, info.comm);
    
    bpf_tail_call(ctx, &prog_array, 0);
    
    bpf_printk("Tail call failed for PID %u (%s)!\n", info.pid, info.comm);
    return 0;
}

char LICENSE[] SEC("license") = "GPL";