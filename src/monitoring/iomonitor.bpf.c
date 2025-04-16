/*
This is the eBPF program that will be compiled into an object file and loaded by the monitoring module. (Compile with clang using a command such as:
clang -O2 -target bpf -c src/monitoring/iomonitor.bpf.c -o src/monitoring/iomonitor.bpf.o)
*/
#include <linux/bpf.h>
#include <bpf/bpf_helpers.h>
#include <linux/ptrace.h>

// Define a BPF map to count sys_read invocations.
// The map is an array with one element indexed by key 0.
struct {
    __uint(type, BPF_MAP_TYPE_ARRAY);
    __uint(max_entries, 1);
    __type(key, u32);
    __type(value, u64);
} io_metrics SEC(".maps");

// Attach this program as a kprobe to the sys_read kernel function.
SEC("kprobe/sys_read")
int kprobe_sys_read(struct pt_regs *ctx) {
    u32 key = 0;
    u64 *value = bpf_map_lookup_elem(&io_metrics, &key);
    if (value) {
        __sync_fetch_and_add(value, 1);
    }
    return 0;
}

char _license[] SEC("license") = "GPL";
