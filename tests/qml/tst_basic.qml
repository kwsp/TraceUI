import QtQuick
import QtTest

// We test basic properties and Style injection validity without loading TraceUI fully 
// if it causes executable-module coupling issues, or we just test independent logic.
TestCase {
    name: "BasicUITest"

    function test_math_sanity() {
        verify(1 + 1 === 2, "Math is broken");
    }
    
    function test_system_monitor_injected() {
        verify(systemMonitor !== undefined, "SystemMonitor context property not found")
        verify(systemMonitor.ramTotalMB >= 0, "SystemMonitor data binding broken")
    }
    
    function test_process_watcher_injected() {
        verify(processWatcher !== undefined, "ProcessWatcher context property not found")
    }
    
    function test_network_monitor_injected() {
        verify(networkMonitor !== undefined, "NetworkMonitor context property not found")
    }
}
