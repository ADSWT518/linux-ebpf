# Build Linux Kernel with eBPF

> 通过 QEMU 虚拟机+自定义内核的方式，实现开发测试的最简化。

假设目前的操作目录为 `LINUX_EBPF` 。

## Step 1: Build Linux kernel

### 1.1 Install build tools

LLVM 工具链用于编译， pahole 用于生成 Debug 信息。

```shell
pacman -S clang llvm lld pahole
```

### 1.2 Get the code

```shell
cd $LINUX_EBPF
git submodule update --init --recursive 
cd linux
```

### 1.3 Configuration

```shell
make defconfig # 默认配置
make menuconfig # 配置菜单

# 打开这5个选项,如果其中有选项找不到说明前面的步骤有问题
# General Setup
# --> BPF subsystem ** eBPF 必备 **
#     --> Enable bpf() system call
#     --> Enable BPF Just In Time compiler
# Kernel hacking ** Debug 信息 **
# --> Compile-time checks and compiler options
#     --> Debug information
#     --> Generate BTF type information
# File systems ** 适配 ArchLinux 镜像 **
# --> Btrfs filesystem support
```

### 1.4 Build

```shell
make LLVM=1 -j$(nproc) # 多线程编译
# 后续选项全都默认即可
```

### 1.5 Generate clangd config (optional)

这一步是为了能够在 vscode 中阅读代码。

```shell
python ./scripts/clang-tools/gen_compile_commands.py
# 会生成一个 compile_commands.json 文件
```

然后在 vscode 中安装 clangd 插件，再打开 linux 文件夹下任意一个C语言文件，就会自动根据这个 json 文件生成 clangd 缓存，然后就可以正常跳转了。

## Step 2. Run Linux with custom kernel

### 2.1 Install QEMU

```shell
pacman -S qemu-full
```

### 2.2 Run the script

```shell
cd $LINUX_EBPF
./start-vm.sh
```

然后就进入到虚拟机了。虚拟机用户名和密码均为 `arch` 。

### 2.3 Set auto-mount

```shell
# 只需要运行一次手动 mount
sudo mount -t 9p -o trans=virtio,version=9p2000.L host_share ~/shared

sudo cp ~/shared/fstab /etc/fstab
```

## Step 3. Compile and verify the eBPF code

我们实际上只需要走到 eBPF verifier 这一步，不需要真的运行。

### 3.1 Compile eBPF code

在宿主机上编译。这里提供了两种测试eBPF code的方式：

1. `test.c` 中直接构造 eBPF 指令并调用 bpf system call 来加载。
2. `test.bpf.c` 中使用 BPF helper function 来编写程序，再用 bpftool 进行加载。（这里我们给出编译好的 bpftool ）

```shell
cd $LINUX_EBPF
cd sharef_files/ebpf_code
make
```

### 3.2 Run the eBPF code

在虚拟机上运行。由于 verify 是 load 之前完成的步骤，所以只要 load 成功则说明通过 verifier ，否则会有 verifier 报错。

```shell
cd ~/shared/ebpf_code

# 方式1
sudo ./test_exec

# 方式2
sudo ./bpftool prog attach ./test.bpf.o /sys/fs/bpf/test
sudo rm /sys/fs/bpf/test
```
