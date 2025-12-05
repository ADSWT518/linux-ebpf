# 获取所有匹配的test前缀
STEMS := $(patsubst %_kern.c,%,$(wildcard *_kern.c))

# 定义所有要生成的文件
OBJECTS := $(addsuffix _kern.o,$(STEMS))
SKELETONS := $(addsuffix _kern.skel.h,$(STEMS))
EXECUTABLES := $(addsuffix _user_exec,$(STEMS))

# 默认目标：构建所有文件
all: $(EXECUTABLES)

# 显式声明所有目标文件
$(OBJECTS): %_kern.o: %_kern.c
	clang -g -O2 -target bpf -c $< -o $@

$(SKELETONS): %_kern.skel.h: %_kern.o
	bpftool gen skeleton $< > $@

$(EXECUTABLES): %_user_exec: %_user.c %_kern.skel.h
	gcc -g -O2 -Wall $< -lbpf -o $@

# 清理所有生成的文件
clean:
	rm -f $(OBJECTS) $(SKELETONS) $(EXECUTABLES)

.PHONY: all clean