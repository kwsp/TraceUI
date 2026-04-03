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
target_include_directories(TraceUIBackend PUBLIC ../src)
target_link_libraries(TraceUIBackend PRIVATE Qt6::Core)
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

# QML UI Tests
qt_add_executable(test_ui test_ui.cpp)
target_link_libraries(test_ui PRIVATE Qt6::Core Qt6::Qml Qt6::QuickTest TraceUIBackend)
set_property(TARGET test_ui PROPERTY AUTOMOC ON)
target_compile_definitions(test_ui PRIVATE QUICK_TEST_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/qml")
add_test(NAME test_ui COMMAND test_ui)
