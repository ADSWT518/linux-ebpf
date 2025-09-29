#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <linux/bpf.h>
#include <sys/syscall.h>

/*
 * 用于bpf()系统调用的一个简单封装
 * __NR_bpf 是 bpf 系统调用的编号
 */
static inline int bpf(int cmd, union bpf_attr *attr, unsigned int size) {
    return syscall(__NR_bpf, cmd, attr, size);
}

// 定义一个足够大的日志缓冲区来接收内核验证器的信息
#define LOG_BUF_SIZE 65536

int main() {
    // 1. 定义eBPF指令数组
    // 这个程序非常简单：
    //   - 将立即数 1 移动到 R0 寄存器 (mov r0, 1)
    //   - 程序退出，并返回 R0 中的值 (exit)
    struct bpf_insn prog[] = {
        // BPF_MOV64_IMM(dst_reg, imm)
        { .code = BPF_ALU64 | BPF_K | BPF_MOV, .dst_reg = BPF_REG_0, .src_reg = 0, .off = 0, .imm = 1 },
        // BPF_EXIT_INSN()
        { .code = BPF_JMP | BPF_EXIT, .dst_reg = 0, .src_reg = 0, .off = 0, .imm = 0 },
    };
    
    // 内核要求所有eBPF程序都必须有关联的许可证
    const char *license = "GPL";
    
    // 用于存储内核验证器日志的缓冲区
    char log_buf[LOG_BUF_SIZE];
    memset(log_buf, 0, sizeof(log_buf));

    // 2. 准备 bpf_attr 结构体以供系统调用
    union bpf_attr attr = {
        .prog_type = BPF_PROG_TYPE_SOCKET_FILTER, // 指定程序类型，这里用套接字过滤器作为例子
        .insn_cnt = sizeof(prog) / sizeof(prog[0]), // 指令数量
        .insns = (unsigned long)prog, // 指向指令数组的指针
        .license = (unsigned long)license, // 指向许可证字符串的指针
        .log_buf = (unsigned long)log_buf, // 指向日志缓冲区的指针
        .log_size = LOG_BUF_SIZE, // 日志缓冲区大小
        .log_level = 1, // 请求详细的验证器日志
    };

    printf("正在加载eBPF程序...\n");

    // 3. 调用 bpf() 系统调用加载程序
    int prog_fd = bpf(BPF_PROG_LOAD, &attr, sizeof(attr));

    // 4. 检查结果
    if (prog_fd < 0) {
        perror("bpf(BPF_PROG_LOAD)失败");
        printf("------ Kernel Verifier Log ------\n");
        printf("%s\n", log_buf);
        printf("---------------------------------\n");
        return 1;
    }

    printf("eBPF程序加载成功！ 程序文件描述符 (Program FD) = %d\n", prog_fd);

    // 在实际应用中，你会使用这个 prog_fd 将程序附加到某个挂钩点
    // (例如，一个套接字)。在这里，我们只是简单地关闭它。
    close(prog_fd);

    return 0;
}