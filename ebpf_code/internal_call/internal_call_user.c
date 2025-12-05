// tail_call_user.c
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <bpf/libbpf.h>
#include "internal_call_kern.skel.h" // 由 bpftool 生成的头文件

static volatile bool exiting = false;

static void sig_handler(int sig) {
    exiting = true;
}

int main(int argc, char **argv) {
    struct internal_call_kern *skel;
    int err = 0;

    // 1. 打开、加载并验证 BPF 程序骨架
    skel = internal_call_kern__open_and_load();
    if (!skel) {
        fprintf(stderr, "Failed to open and load BPF skeleton\n");
        return 1;
    }

    // 3. 附加 tracepoint 程序
    err = internal_call_kern__attach(skel);
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
    internal_call_kern__destroy(skel);
    printf("\nBPF programs detached and unloaded.\n");
    return err < 0 ? -err : 0;
}