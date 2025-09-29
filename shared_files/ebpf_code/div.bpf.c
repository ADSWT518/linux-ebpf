#include <linux/bpf.h>
#include <bpf/bpf_helpers.h>


char LICENSE[] SEC("license") = "GPL";

// Map 值的结构体，大小为 8 字节
struct my_data {
    char buffer[8];
};

// 定义一个数组映射
struct {
    __uint(type, BPF_MAP_TYPE_ARRAY);
    __uint(max_entries, 1);
    __type(key, __u32); // 使用标准类型
    __type(value, struct my_data);
} my_map SEC(".maps");


SEC("kprobe/__x64_sys_clone")
int map_access_fail(struct pt_regs *ctx) {
    __u32 key = 0;
    struct my_data *data_ptr;

    // 从 Map 中查找元素
    data_ptr = bpf_map_lookup_elem(&my_map, &key);
    if (!data_ptr) {
        return 0;
    }
    // 验证器此时知道 data_ptr 指向一块大小精确为 8 字节的内存区域。

    // 2. 使用 bpf_get_prandom_u32() 获取两个初始未知的变量
    __u64 x = bpf_get_prandom_u32() % 10;
    __u64 y = bpf_get_prandom_u32() % 10;

    if (x < 1 || x > 3) {
        bpf_printk("x not in [1,3]");
        return 0;
    }

    if (y < 2 || y > 7) {
        bpf_printk("x not in [2,7]");
        return 0;
    }

    __u64 res = y / x;
    bpf_printk("x = %llu, y = %llu, res = %llu", x, y, res);

    // 触发验证器错误
    char value = data_ptr->buffer[res];
    
    bpf_printk("Value read: %d", value);

    return 0;
}