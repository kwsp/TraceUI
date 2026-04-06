qt6_add_library(TraceUITerminal STATIC)
qt6_add_qml_module(TraceUITerminal
    URI TraceUI.Terminal
    VERSION 1.0
    QML_FILES
        TerminalPanel.qml
    SOURCES
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
