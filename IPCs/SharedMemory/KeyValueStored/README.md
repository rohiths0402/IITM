# Interpreting `perf` Output for Key-Value Store

You can profile your key-value store using `perf` to monitor syscall activity on both the server and client sides.

### Commands
```bash
# Server profiling
sudo perf stat -e syscalls:sys_enter_read,syscalls:sys_enter_write,syscalls:sys_enter_accept ./server

# Client profiling
sudo perf stat -e syscalls:sys_enter_read,syscalls:sys_enter_write,syscalls:sys_enter_accept ./client
```

---

## Example Output (Server)

```
 Performance counter stats for './server':

               202      syscalls:sys_enter_read
               203      syscalls:sys_enter_write
                 2      syscalls:sys_enter_accept

      74.616471946 seconds time elapsed
```

### Server-Side Interpretation

| **Observation**  |                           **Meaning**                           |               **Inference**             |
|----------------- |-----------------------------------------------------------------|-----------------------------------------|
| read = 202       | The server performed 37 read operations from connected clients. | Low load; likely few requests received. |
| write = 203      | Only 3 responses sent to clients.                               | Indicates limited interaction or mostly idle server. |
| accept = 2       | Two client connections accepted.                                | Few clients connected during profiling. |
| **Overall**      | Low syscall counts and high elapsed time (97s).                 | Server was mostly waiting for client requests or idle. |

> If these counts were much higher (e.g., 3700 reads, 300 writes), it would indicate a busy server actively serving multiple concurrent requests.

---

## Example Output (Client)

```
P Performance counter stats for './client':

               203      syscalls:sys_enter_read
               402      syscalls:sys_enter_write
                 0      syscalls:sys_enter_accept

       0.064291496 seconds time elapsed
```

### Client-Side Interpretation


| **Observation**  |                           **Meaning**                           |               **Inference**             |
|----------------- |-----------------------------------------------------------------|-----------------------------------------|
| write = 402 | Client issued many write calls (sending commands/requests).        | Indicates client actively communicating with the server. |
| read = 203 | Client received fewer responses than sent requests. | May be waiting for replies or batching requests. |
| accept = 2 | Two connection attempts made. | Could be reconnects or multiple sessions. |
| **Overall** | More writes than reads. | Client is request-heavy; may be generating load. |

---

## Syscall Bottleneck Interpretation Summary

| **Syscall** | **When Count is High** | **Likely System State** | **Possible Fix / Optimization** |
|--------------|------------------------|---------------------------|----------------------------------|
| `read()` | Frequent small reads | I/O-bound, waiting for data | Use larger buffers, batch reads, or non-blocking I/O |
| `write()` | Many small writes | Kernel overhead due to syscall transitions | Buffer or batch writes, use async I/O |
| `accept()` | Many short-lived connections | Connection churn, context switches | Use connection pooling, reuse sockets |
| `close()` | Frequent open/close cycles | Descriptor churn, possible exhaustion | Keep connections alive longer |
| `connect()` | Many connection attempts | Handshake overhead | Use persistent connections or keep-alive |
| `send()` / `recv()` | High message exchange frequency | Heavy socket I/O | Batch messages, reduce fragmentation |
| `fsync()` | Frequent disk flushes | I/O wait for durability | Group commits, delayed fsync, async WAL flush |

---

## How to Interpret Patterns

| **Pattern** | **Meaning** | **System State** | **Fix / Action** |
|--------------|-------------|------------------|------------------|
| `read()` ≫ `write()` | Server mostly waiting for input | I/O wait / underutilized | Use non-blocking reads, increase client concurrency |
| `write()` ≫ `read()` | Writes dominate | Output-heavy / possible backpressure | Buffer writes, apply flow control |
| `accept()` and `close()` both high | Frequent short connections | Connection churn | Reuse connections, enable persistent sessions |
| All syscall counts very high | Too many kernel transitions | Kernel-bound workload | Batch I/O, use asynchronous operations |
| Low syscall counts overall | Idle or lightly loaded system | Underutilized | Increase request rate to stress test properly |

---

### ✅ Summary
Using `perf` on your key-value store helps identify performance bottlenecks at the syscall level.  
By analyzing the counts of `read`, `write`, `accept`, `connect`, and `close` syscalls, you can infer server load, client activity, and opportunities for optimization such as batching, connection pooling, or asynchronous I/O.
