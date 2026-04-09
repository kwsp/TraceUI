qt6_add_library(TraceUITerminal STATIC)
qt6_add_qml_module(TraceUITerminal
    URI TraceUITerminal
    VERSION 1.0
    QML_FILES
        TerminalPanel.qml
    SOURCES
        TerminalBackend.h
        TerminalBackend.cpp
        TerminalRenderer.h
        TerminalRenderer.cpp
        GlyphMaterial.h
        GlyphMaterial.cpp
        FileDescriptor.h
        FileDescriptor.cpp
)

target_include_directories(TraceUITerminal PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src
)

find_package(Qt6 REQUIRED COMPONENTS ShaderTools)

qt6_add_shaders(TraceUITerminal "glyph_shaders"
    PREFIX
        "/terminal"
    FILES
        shaders/glyph.vert
        shaders/glyph.frag
)

target_link_libraries(TraceUITerminal PUBLIC
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    vterm
)
