// SPDX-License-Identifier: GPL-2.0
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <bpf/libbpf.h>
#include <bpf/bpf.h>
#include "bench_call_kern.skel.h"

static double get_time_ns(void)
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return (double)ts.tv_sec * 1e9 + ts.tv_nsec;
}

int main(int argc, char **argv)
{
	struct bench_call_kern *skel;
	int err, prog_fd;
	char data_in[128] = {};
	double start_ns, end_ns, duration_sec;

	skel = bench_call_kern__open_and_load();
	if (!skel) {
		fprintf(stderr, "Failed to load skeleton\n");
		return 1;
	}
	prog_fd = bpf_program__fd(skel->progs.bench_bpf_call);

	LIBBPF_OPTS(bpf_test_run_opts, opts,
		.data_in = &data_in,
		.data_size_in = sizeof(data_in),
		.repeat = 1000000, 
	);

	printf("Warming up...\n");
	bpf_prog_test_run_opts(prog_fd, &opts);

	printf("Running benchmark (1,000,000 runs * 1000 calls = 1 Billion calls)...\n");

	start_ns = get_time_ns();
	err = bpf_prog_test_run_opts(prog_fd, &opts);
	end_ns = get_time_ns();

	if (err != 0) {
		fprintf(stderr, "test_run failed: %d\n", err);
		bench_call_kern__destroy(skel);
		return 1;
	}

	duration_sec = (end_ns - start_ns) / 1e9;
	printf("======================================\n");
	printf("Total elapsed time: %.6f seconds\n", duration_sec);
	printf("Throughput:         %.2f Billion calls/sec\n", 1.0 / duration_sec);
	printf("Time per call:      %.4f ns\n", (end_ns - start_ns) / 1000000000.0);
	printf("======================================\n");

	bench_call_kern__destroy(skel);
	return 0;
}