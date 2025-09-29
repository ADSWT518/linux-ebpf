#!/bin/bash

# 获取脚本所在目录作为工作目录
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

# 内核源码路径
KERNEL_DIR=$DIR/linux

# 磁盘镜像路径
DISK_IMG=$DIR/Arch-Linux-x86_64-basic.qcow2

SHARED_DIR=$DIR/shared_files

# 如果磁盘镜像不存在，则下载
if [ ! -f "$DISK_IMG" ]; then
    echo "未找到磁盘镜像，正在下载..."
    MIRROR_URL="https://mirror.zju.edu.cn/archlinux/images/latest/Arch-Linux-x86_64-basic.qcow2"
    
    # 使用wget下载（支持断点续传）
    if command -v wget &> /dev/null; then
        wget -c -O "$DISK_IMG" "$MIRROR_URL"
    # 如果wget不可用，尝试使用curl
    elif command -v curl &> /dev/null; then
        curl -C - -o "$DISK_IMG" "$MIRROR_URL"
    else
        echo "错误：需要wget或curl来下载镜像"
        exit 1
    fi
    
    # 检查下载是否成功
    if [ ! -f "$DISK_IMG" ]; then
        echo "下载失败！请检查网络连接或手动下载镜像"
        exit 1
    fi
fi

# 确保共享目录存在
mkdir -p "$SHARED_DIR"

# 启动QEMU
qemu-system-x86_64 \
  -m 4G \
  -smp 4 \
  -kernel "${KERNEL_DIR}/arch/x86/boot/bzImage" \
  -drive file="${DISK_IMG}",format=qcow2,if=virtio \
  -append "root=/dev/vda3 rw console=ttyS0 audit=0" \
  -nographic \
  -enable-kvm \
  -fsdev local,id=fsdev0,path="${SHARED_DIR}",security_model=passthrough \
  -device virtio-9p-pci,fsdev=fsdev0,mount_tag=host_share \
  -netdev user,id=net0,hostfwd=tcp::2222-:22 \
  -device virtio-net-pci,netdev=net0