find_package(Qt6 REQUIRED COMPONENTS Core Test Qml QuickTest)

if(NOT APPLE)
    message(STATUS "Tests are currently macOS only due to hardware APIs.")
    return()
endif()

# Create a static library for backend components so tests can link against it
add_library(TraceUIBackend STATIC
    ../src/backend/SystemMonitor.cpp
    ../src/backend/macos/SystemMonitorMacOS.cpp
    ../src/backend/ProcessWatcher.cpp
    ../src/backend/ProcessListModel.cpp
    ../src/backend/macos/ProcessWatcherMacOS.cpp
    ../src/backend/NetworkMonitor.cpp
    ../src/backend/macos/NetworkMonitorMacOS.cpp
)

if(BUILD_TERMINAL)
    target_link_libraries(TraceUIBackend PRIVATE TraceUITerminal)
endif()

target_include_directories(TraceUIBackend PUBLIC ../src)
target_link_libraries(TraceUIBackend PRIVATE Qt6::Core Qt6::Gui Qt6::Qml)

if(APPLE)
    target_link_libraries(TraceUIBackend PRIVATE "-framework CoreFoundation")
endif()
set_property(TARGET TraceUIBackend PROPERTY AUTOMOC ON)

# SystemMonitor Test
qt_add_executable(test_SystemMonitor test_SystemMonitor.cpp)
target_link_libraries(test_SystemMonitor PRIVATE Qt6::Core Qt6::Test TraceUIBackend)
set_property(TARGET test_SystemMonitor PROPERTY AUTOMOC ON)
add_test(NAME test_SystemMonitor COMMAND test_SystemMonitor)

# ProcessWatcher Test
qt_add_executable(test_ProcessWatcher test_ProcessWatcher.cpp)
target_link_libraries(test_ProcessWatcher PRIVATE Qt6::Core Qt6::Test TraceUIBackend)
set_property(TARGET test_ProcessWatcher PROPERTY AUTOMOC ON)
add_test(NAME test_ProcessWatcher COMMAND test_ProcessWatcher)

# NetworkMonitor Test
qt_add_executable(test_NetworkMonitor test_NetworkMonitor.cpp)
target_link_libraries(test_NetworkMonitor PRIVATE Qt6::Core Qt6::Test TraceUIBackend)
set_property(TARGET test_NetworkMonitor PROPERTY AUTOMOC ON)
add_test(NAME test_NetworkMonitor COMMAND test_NetworkMonitor)

# HistoryProvider Test
qt_add_executable(test_HistoryProvider test_HistoryProvider.cpp
    ../src/backend/ScrollingHistoryProvider.cpp
    ../src/backend/CpuHistoryProvider.cpp
    ../src/backend/NetworkHistoryProvider.cpp
)
target_include_directories(test_HistoryProvider PRIVATE ../src)
target_link_libraries(test_HistoryProvider PRIVATE Qt6::Core Qt6::Gui Qt6::Quick Qt6::Test)
set_property(TARGET test_HistoryProvider PROPERTY AUTOMOC ON)
add_test(NAME test_HistoryProvider COMMAND test_HistoryProvider)

# QML UI Tests
qt_add_executable(test_ui test_ui.cpp)
target_link_libraries(test_ui PRIVATE Qt6::Core Qt6::Qml Qt6::QuickTest TraceUIBackend)
set_property(TARGET test_ui PROPERTY AUTOMOC ON)
target_compile_definitions(test_ui PRIVATE QUICK_TEST_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/qml")
add_test(NAME test_ui COMMAND test_ui)

# Terminal Test
if(BUILD_TERMINAL)
    qt_add_executable(test_Terminal test_Terminal.cpp)
    target_include_directories(test_Terminal PRIVATE ../src)
    target_link_libraries(test_Terminal PRIVATE Qt6::Core Qt6::Test TraceUIBackend TraceUITerminal)
    set_property(TARGET test_Terminal PROPERTY AUTOMOC ON)
    add_test(NAME test_Terminal COMMAND test_Terminal)

    qt_add_executable(test_TerminalRenderer test_TerminalRenderer.cpp)
    target_include_directories(test_TerminalRenderer PRIVATE ../src)
    target_link_libraries(test_TerminalRenderer PRIVATE Qt6::Core Qt6::Quick Qt6::Test TraceUITerminal)
    set_property(TARGET test_TerminalRenderer PROPERTY AUTOMOC ON)
    add_test(NAME test_TerminalRenderer COMMAND test_TerminalRenderer)
endif()
