# System & Network Monitoring Architecture

TraceUI uses a provider-based architecture to fetch real-time system metrics via platform-specific APIs. On macOS, this is implemented using `sysctl`, `mach` ports, `libproc`, and `IOKit`.

## Core Components

### SystemMonitor
Exposes hardware and OS metrics via QML properties.
- **CPU Usage**: Fetched via `host_processor_info` (Mach). Tracks user/system/idle ticks per core.
- **Memory**: Fetched via `host_statistics64` (Mach). Aggregates active, wired, and compressed pages.
- **CPU Temp**: Fetched via `AppleSMC` (IOKit). Decodes `flt ` (4-byte IEEE float) for Apple Silicon and `sp78` (2-byte fixed-point) for Intel.
- **Power Source**: Fetched via `IOPowerSources` (IOKit). Reports "AC" or "BAT %".
- **Uptime**: Derived from `KERN_BOOTTIME` (sysctl) against current system time.

### NetworkMonitor
Exposes interface and traffic metrics.
- **Traffic Throughput**: Samples `if_data` (getifaddrs) every 2 seconds to calculate bytes/sec.
- **IPv4 Detection**: Scans `ifaddrs` linked list for the first non-loopback `AF_INET` interface.
- **Active Connections**: Samples `net.inet.tcp.pcbcount` (sysctl) for system-wide TCP state.
- **Latency/Loss**: Spawns async `ping -c 1` (QProcess) targeting `1.1.1.1`. Uses a sliding window (20 samples) for packet loss percentage.

### ProcessWatcher
Provides a filtered list of top processes.
- **Data Source**: Fetched via `KERN_PROC_ALL` (sysctl).
- **CPU Attribution**: Calculates delta of `pti_total_user` + `pti_total_system` (libproc) against high-resolution `pollTimer` ticks. Normalized by core count.
- **Memory Attribution**: Reads `pti_resident_size`.

## Platform APIs Used (macOS)

| Metric | API / Sysctl | Header |
| :--- | :--- | :--- |
| Core Load | `PROCESSOR_CPU_LOAD_INFO` | `<mach/mach_host.h>` |
| CPU Temp | `AppleSMC` UserClient | `<IOKit/IOKitLib.h>` |
| Traffic | `getifaddrs` / `if_data` | `<ifaddrs.h>` |
| Conn Count | `net.inet.tcp.pcbcount` | `<sys/sysctl.h>` |
| Proc Info | `proc_pidinfo` | `<libproc.h>` |
| Power | `IOPowerSources` | `<IOKit/ps/IOPowerSources.h>` |

## Performance Considerations
- **Polling**: System metrics poll at 1Hz; Network metrics at 0.5Hz.
- **Non-blocking**: Latency checks (ping) run in a separate process via `QProcess` to avoid blocking the main event loop.
- **Caching**: Process names are cached by PID to reduce `proc_pidpath` syscall overhead.
