// tail_call_kern.c

#include <linux/bpf.h>
#include <bpf/bpf_helpers.h>

// 定义一个结构体，用于在 Map 中存储输入参数和结果
struct data_t {
    __u64 a;
    __u64 b;
    __u64 result;
};

// 用于尾调用的程序数组 Map，保持不变
struct {
    __uint(type, BPF_MAP_TYPE_PROG_ARRAY);
    __uint(max_entries, 10);
    __uint(key_size, sizeof(__u32));
    __uint(value_size, sizeof(__u32));
} prog_array SEC(".maps");

// 新增的 Map，用于传递数据
// 使用 PERCPU_ARRAY 可以为每个 CPU 创建一个独立的存储空间，避免加锁
struct {
    __uint(type, BPF_MAP_TYPE_PERCPU_ARRAY);
    __uint(max_entries, 1); // 我们只需要一个逻辑条目，内核会为每个CPU复制一份
    __uint(key_size, sizeof(__u32));
    __uint(value_size, sizeof(struct data_t));
} data_map SEC(".maps");


// bpf_prog2: "被调用者"，负责计算
SEC("tracepoint")
int bpf_prog2(void *ctx) {
    __u32 data_key = 0;
    struct data_t *data;

    // 从 Map 中查找当前 CPU 对应的存储单元
    data = bpf_map_lookup_elem(&data_map, &data_key);
    if (!data) {
        // 理论上不应该发生，但必须有此检查以通过验证器
        return 0;
    }

    // 执行计算，并将结果写回 Map
    data->result = data->a + data->b;

    bpf_printk("Prog 2: Calculation done. %llu + %llu = %llu\n", data->a, data->b, data->result);

    return 0;
}

// bpf_prog1: "调用者"，负责设置参数并发起尾调用
SEC("tp/syscalls/sys_enter_newuname")
int bpf_prog1(void *ctx) {
    __u32 key = 0;
    struct data_t *data;

    // 从 Map 中查找当前 CPU 对应的存储单元
    data = bpf_map_lookup_elem(&data_map, &key);
    if (!data) {
        return 0;
    }

    // "传递参数"：将输入值写入 Map
    data->a = 100;
    data->b = 23;
    data->result = 0; // 清空之前可能存在的结果

    char comm[16];
    bpf_get_current_comm(&comm, sizeof(comm));
    bpf_printk("Prog 1: Triggered by '%s'. Setting params: a=%llu, b=%llu. Preparing tail call...\n", comm, data->a, data->b);

    // 执行尾调用
    bpf_tail_call(ctx, &prog_array, 0);
    // 这里第三个参数改为10即可触发尾调用失败的逻辑
    // bpf_tail_call(ctx, &prog_array, 10);

    // 如果尾调用失败，会执行到这里
    bpf_printk("Tail call failed!\n");
    return 0;
}

char LICENSE[] SEC("license") = "GPL";