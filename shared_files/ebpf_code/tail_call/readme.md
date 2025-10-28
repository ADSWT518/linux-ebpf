# bpf_tail_call

* `tail_call_1` is a program with the most simple tail call, it uses print to indicate whether the tail call succeeds.
* `tail_call_2` is another program using arithmatic addition to indicate whether the tail call succeeds.

## Build

```shell
make
```

## Run

```shell
sudo ./tail_call_1_user_exec
# sudo ./tail_call_2_user_exec
```

然后在另一个终端中观察输出。

```shell
sudo cat /sys/kernel/debug/tracing/trace_pipe
```

## Check the error

目的是触发这个逻辑：

```c
JMP_TAIL_CALL: {
    ...
    if (unlikely(index >= array->map.max_entries))
        goto out;
    ...
}
```

在 `*_kern.c` 文件中，我们对 `prog_array` 设置的 `max_entries=10`。所以只需要在 caller function 中把 `bpf_tail_call(ctx, &prog_array, 0);` 修改为 `bpf_tail_call(ctx, &prog_array, 10);` 即可触发。