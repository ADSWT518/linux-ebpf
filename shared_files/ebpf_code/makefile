# --- 编译器和工具定义 ---
CC = clang
OBJDUMP = llvm-objdump
CFLAGS = -g -O2 # -g 是生成源码级调试信息的关键

# --- 文件和目标定义 ---

# 路径 1: 手动指令构建器
MANUAL_SRC = test.c
MANUAL_EXEC = test_exec

# 路径 2: 自动发现并处理所有 eBPF C 程序
# 查找所有 .bpf.c 源文件
BPF_C_SRCS = $(wildcard *.bpf.c)
# 生成对应的 .bpf.o 目标文件列表
BPF_OBJS = $(BPF_C_SRCS:.c=.o)
# 生成对应的 .bpf.s 汇编文件列表
BPF_ASMS = $(BPF_C_SRCS:.c=.s)

# vmlinux.h 的文件名
VMLINUX_H = vmlinux.h

# --- 内核和 bpftool 设置 ---
KDIR ?= $(shell pwd)/../../linux
VMLINUX_PATH = $(KDIR)/vmlinux

# --- 构建规则 ---

# 默认目标：构建所有内容
all: $(MANUAL_EXEC) $(BPF_OBJS) $(BPF_ASMS)

# 规则 1: 编译手动指令构建器 (test.c)
$(MANUAL_EXEC): $(MANUAL_SRC)
	@echo "  CC      [Manual Builder] $(MANUAL_SRC) -> $@"
	$(CC) $(CFLAGS) $(MANUAL_SRC) -o $(MANUAL_EXEC)

# 规则 2: 编译 eBPF C 代码为 eBPF 字节码 (.o 文件)
%.bpf.o: %.bpf.c $(VMLINUX_H)
	@echo "  BPF-CC  [eBPF Code]      $< -> $@"
	$(CC) $(CFLAGS) -target bpf -I. -c $< -o $@

# 新增规则 3: 从 .o 文件生成汇编文件 (.s 文件)
# 这条规则依赖于对应的 .o 文件，确保编译完成后再进行反汇编
%.bpf.s: %.bpf.o
	@echo "  OBJDUMP [eBPF Assembly]  $< -> $@"
	$(OBJDUMP) -d -S $< > $@

# 规则：生成 vmlinux.h (如果不存在)
$(VMLINUX_H):
	@echo "  GEN     $@"
	@if ! test -f "$(VMLINUX_PATH)"; then \
		echo "ERROR: vmlinux not found at $(VMLINUX_PATH)"; \
		echo "Hint: Ensure your kernel was compiled with CONFIG_DEBUG_INFO_BTF=y"; \
		echo "      Or specify kernel source path with 'make KDIR=/path/to/linux/source'"; \
		exit 1; \
	fi
	bpftool btf dump file $(VMLINUX_PATH) format c > $(VMLINUX_H)

# --- 清理规则 ---
clean:
	@echo "  CLEAN"
	rm -f $(MANUAL_EXEC) $(BPF_OBJS) $(BPF_ASMS) $(VMLINUX_H)

# 将 vmlinux.h 标记为 .SECONDARY，防止 make 自动删除它
.SECONDARY: $(VMLINUX_H)

# 声明伪目标
.PHONY: all clean