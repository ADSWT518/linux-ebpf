// tail_call_user.c

#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <sys/resource.h>
#include <bpf/libbpf.h>
#include "tail_call_2_kern.skel.h" // 包含新生成的骨架文件

// 定义与内核态一致的结构体
struct data_t {
    __u64 a;
    __u64 b;
    __u64 result;
};

static volatile bool exiting = false;

static void sig_handler(int sig) {
    exiting = true;
}

static int libbpf_print_fn(enum libbpf_print_level level, const char *format, va_list args) {
    return vfprintf(stderr, format, args);
}

int main(int argc, char **argv) {
    struct tail_call_2_kern *skel;
    int err;
    int prog_array_fd, data_map_fd;
    int prog2_fd;
    int key = 0;

    libbpf_set_print(libbpf_print_fn);

    // 获取系统上可能存在的CPU数量
    int num_cpus = libbpf_num_possible_cpus();
    if (num_cpus < 0) {
        fprintf(stderr, "Failed to get number of possible CPUs\n");
        return 1;
    }
    // 为每个CPU的结果分配空间
    struct data_t values[num_cpus];

    skel = tail_call_2_kern__open();
    if (!skel) {
        fprintf(stderr, "Failed to open BPF skeleton\n");
        return 1;
    }

    err = tail_call_2_kern__load(skel);
    if (err) {
        fprintf(stderr, "Failed to load BPF skeleton\n");
        goto cleanup;
    }

    // 找到两个 Map 的文件描述符
    prog_array_fd = bpf_map__fd(skel->maps.prog_array);
    data_map_fd = bpf_map__fd(skel->maps.data_map);
    if (prog_array_fd < 0 || data_map_fd < 0) {
        fprintf(stderr, "Failed to find map FDs\n");
        goto cleanup;
    }

    // 找到 bpf_prog2 的文件描述符
    prog2_fd = bpf_program__fd(skel->progs.bpf_prog2);
    if (prog2_fd < 0) {
        fprintf(stderr, "Failed to find program bpf_prog2 FD\n");
        goto cleanup;
    }

    // 将 bpf_prog2 添加到程序数组中，以便尾调用可以找到它
    err = bpf_map__update_elem(skel->maps.prog_array, &key, sizeof(key), &prog2_fd, sizeof(prog2_fd), BPF_ANY);
    if (err) {
        fprintf(stderr, "Failed to update prog_array map: %s\n", strerror(errno));
        goto cleanup;
    }

    err = tail_call_2_kern__attach(skel);
    if (err) {
        fprintf(stderr, "Failed to attach BPF skeleton\n");
        goto cleanup;
    }

    signal(SIGINT, sig_handler);
    signal(SIGTERM, sig_handler);

    printf("Successfully started! Please run `sudo cat /sys/kernel/debug/tracing/trace_pipe` to see kernel prints.\n");
    printf("Press Ctrl-C to exit.\n\n");
    printf("Waiting for calculation results from BPF...\n");

    while (!exiting) {
        sleep(2); // 每2秒检查一次结果

        int data_key = 0;
        // 从 PERCPU_ARRAY Map 中读取所有CPU的值
        err = bpf_map__lookup_elem(skel->maps.data_map, &data_key, sizeof(data_key), values, sizeof(values), 0);
        if (err) {
            fprintf(stderr, "Failed to lookup data map: %s\n", strerror(errno));
            continue;
        }

        // 遍历每个CPU的结果
        for (int i = 0; i < num_cpus; i++) {
            // 如果 result 字段不为0，说明这个CPU上发生过一次计算
            if (values[i].result != 0) {
                printf("Result from CPU %d: %llu + %llu = %llu\n",
                       i, values[i].a, values[i].b, values[i].result);
                
                // (可选) 清除结果，以便下次只显示新的计算
                // values[i].result = 0; 
                // bpf_map_update_elem(data_map_fd, &key, values, BPF_ANY);
            }
        }
    }

cleanup:
    tail_call_2_kern__destroy(skel);
    return -err;
}