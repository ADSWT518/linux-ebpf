// tail_call_user.c
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <bpf/libbpf.h>
#include "tail_call_1_kern.skel.h" // 由 bpftool 生成的头文件

static volatile bool exiting = false;

static void sig_handler(int sig) {
    exiting = true;
}

int main(int argc, char **argv) {
    struct tail_call_1_kern *skel;
    int err = 0;

    // 1. 打开、加载并验证 BPF 程序骨架
    skel = tail_call_1_kern__open_and_load();
    if (!skel) {
        fprintf(stderr, "Failed to open and load BPF skeleton\n");
        return 1;
    }

    // 2. 设置尾调用 map
    // 获取被调用程序 bpf_prog2 的文件描述符
    int prog2_fd = bpf_program__fd(skel->progs.bpf_prog2);
    if (prog2_fd < 0) {
        fprintf(stderr, "Failed to get bpf_prog2 fd: %d\n", prog2_fd);
        goto cleanup;
    }

    // 将 bpf_prog2 的文件描述符更新到 prog_array map 的索引 0 位置
    int index = 0;
    err = bpf_map__update_elem(skel->maps.prog_array, &index, sizeof(index), &prog2_fd, sizeof(prog2_fd), BPF_ANY);
    if (err) {
        fprintf(stderr, "Failed to update prog_array map: %s\n", strerror(-err));
        goto cleanup;
    }

    // 3. 附加 tracepoint 程序
    err = tail_call_1_kern__attach(skel);
    if (err) {
        fprintf(stderr, "Failed to attach BPF skeleton: %d\n", err);
        goto cleanup;
    }

    printf("BPF programs loaded and attached successfully!\n");
    printf("Run `uname -a` in another terminal to trigger the programs.\n");
    printf("Watch the output in `sudo cat /sys/kernel/debug/tracing/trace_pipe`.\n");
    printf("Press Ctrl-C to exit.\n");

    // 设置信号处理，以便优雅退出
    signal(SIGINT, sig_handler);
    signal(SIGTERM, sig_handler);

    // 循环等待，直到用户按下 Ctrl-C
    while (!exiting) {
        sleep(1);
    }

cleanup:
    // 销毁骨架，卸载 BPF 程序和 map
    tail_call_1_kern__destroy(skel);
    printf("\nBPF programs detached and unloaded.\n");
    return err < 0 ? -err : 0;
}