add_library(TraceUITerminal STATIC
    TerminalBackend.h
    TerminalBackend.cpp
    TerminalModel.h
    TerminalModel.cpp
    FileDescriptor.h
    FileDescriptor.cpp
)

target_include_directories(TraceUITerminal PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(TraceUITerminal PUBLIC
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    vterm
)

set_property(TARGET TraceUITerminal PROPERTY AUTOMOC ON)
