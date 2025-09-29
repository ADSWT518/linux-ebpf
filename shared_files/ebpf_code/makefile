# Makefile for comparing manual eBPF instruction building vs. libbpf-based compilation

# --- 编译器和通用设置 ---
CC = clang
CFLAGS = -g -O2

# --- 文件和目标定义 ---

# 路径 1: 手动指令构建器
MANUAL_SRC = test.c
MANUAL_EXEC = test_exec

# 路径 2: 现代 eBPF C 程序
BPF_C_SRC = test.bpf.c
BPF_OBJ = $(BPF_C_SRC:.c=.o)

# vmlinux.h 的文件名
VMLINUX_H = vmlinux.h

# --- 内核和 bpftool 设置 ---
# 用户可以通过 `make KDIR=/path/to/linux` 来覆盖它
KDIR ?= $(shell pwd)/../../linux
VMLINUX_PATH = $(KDIR)/vmlinux

# --- 构建规则 ---

# 默认目标：构建所有东西，用于对比
all: $(MANUAL_EXEC) $(BPF_OBJ)

# 规则 1: 编译手动指令构建器 (test.c)
# 这是一个标准的 C 程序编译，不需要特殊库
$(MANUAL_EXEC): $(MANUAL_SRC)
	@echo "  CC      [Manual Builder] $(MANUAL_SRC) -> $@"
	$(CC) $(CFLAGS) $(MANUAL_SRC) -o $(MANUAL_EXEC)

# 规则 2: 编译 eBPF C 代码为 eBPF 字节码 (test.bpf.c)
# 这需要 -target bpf，并且依赖 vmlinux.h
$(BPF_OBJ): $(BPF_C_SRC) $(VMLINUX_H)
	@echo "  BPF-CC  [eBPF C Code]    $(BPF_C_SRC) -> $@"
	$(CC) $(CFLAGS) -target bpf -I. -c $(BPF_C_SRC) -o $(BPF_OBJ)

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
	rm -f $(MANUAL_EXEC) $(BPF_OBJ) $(VMLINUX_H)

# 将 vmlinux.h 标记为 .SECONDARY，防止 make 自动删除它
.SECONDARY: $(VMLINUX_H)

# 声明伪目标
.PHONY: all clean