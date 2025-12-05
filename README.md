# Build Linux Kernel with eBPF

> 通过 QEMU 虚拟机+自定义内核的方式，实现开发测试的最简化。
> 基于 `linux/tools/testing/selftest/bpf/vmtest.sh` 和 `linux/.vscode/tasks.sh` 。

假设目前的操作目录为 `LINUX_EBPF` 。

## Step 1: Build & run Linux kernel

### 1.1 Install build & run tools

LLVM 工具链用于编译， pahole 用于生成 Debug 信息。

```shell
pacman -S clang llvm lld pahole qemu-full 
```

### 1.2 Get the code

```shell
cd $LINUX_EBPF
git submodule update --init --recursive 
cd linux
```

### 1.3 Build

1. 用 VSCode 打开 linux 文件夹。
2. 直接 `Ctrl + Shift + B` 。这一步同时也会生成 compile_commands.json 文件。

### 1.4 Run kernel using QEMU

用 VSCode 打开 linux 文件夹，然后直接 `F5` 。

## Step 2. Build & run BPF test

```shell
cd $LINUX_EBPF/linux/tools/testing/selftests/bpf
LDLIBS=-static PKG_CONFIG='pkg-config --static' ./vmtest.sh # 开启静态链接
```

默认会完整运行 test_prog 测试。

* 如果想指定运行 test_prog 中某个文件中的测试

    ```shell
    LDLIBS=-static PKG_CONFIG='pkg-config --static' ./vmtest.sh -- ./test_progs -t verifier_sdiv
    ```

* 如果想运行其他测试

    ```shell
    LDLIBS=-static PKG_CONFIG='pkg-config --static' ./vmtest.sh -- ./test_verifier
    ```

## Step 3. Debug BPF programs

### 3.1 Compile eBPF code

在宿主机上编译。这里提供了三种测试eBPF code的方式：

1. 参考 `ebpf_code/test.c`: 直接构造 eBPF 指令并调用 bpf system call 来加载，这样可以直接获得一个可执行的 `test_exec` 文件。
2. 参考 `ebpf_code/test.bpf.c`: 使用 BPF helper function 来编写程序，这样可以获得一个 `test.bpf.o` 文件，再用 bpftool 进行加载。
```shell
bpftool prog load test.bpf.o /sys/fs/bpf/test
```
3. 参考 `ebpf_code/selftests/internal_call/`: 使用 BPF helper function 编写 `*_kern.c` 程序，然后使用 libbpf 编写 `*_user.c` 程序，最后使用 Makefile 编译。这样可以直接获得一个可执行的 `*_user_exec` 文件。

### 3.2 Run kernel using QEMU

用 VSCode 打开 linux 文件夹，然后直接 `F5` 。如果要打断点的话，直接在 VSCode 里用鼠标点击行号左边。

### 3.3 Run BPF programs

有两种方式可以获取到宿主机上编译好的bpf程序。

1. 在虚拟机上访问共享文件夹

    ```shell
    # 虚拟机上
    cd /host/home/tyz/...
    ```

2. 在宿主机上使用scp

    ```shell
    # 宿主机上
    ./.vscode/tasks.sh push ./tools/testing/selftests/bpf/test_verifier
    # 虚拟机上
    ./test_verifier
    ```

### 3.4 Open another terminal

```shell
# 宿主机上
./.vscode/tasks.sh ssh
```
