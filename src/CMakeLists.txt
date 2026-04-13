find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick QmlModels QuickControls2 ShaderTools)
qt_standard_project_setup(REQUIRES 6.5)

qt_add_executable(${PROJECT_NAME}
  main.cpp
)

set(TRACEUI_QML_SINGLETONS
  Style.qml
  LayoutStore.qml
  AnimConfig.qml
)
set_source_files_properties(${TRACEUI_QML_SINGLETONS}
  PROPERTIES QT_QML_SINGLETON_TYPE TRUE)

set(TRACEUI_QML_FILES
  ${TRACEUI_QML_SINGLETONS}
  TurnOnReveal.qml
  TerminalContainer.qml
  Main.qml
  SystemPanel.qml
  ProcessPanel.qml
  NetworkPanel.qml
  GlobePanel.qml
  TilingPanelWrapper.qml
  SplitDivider.qml
  PanelFrame.qml
  SectionBracket.qml
)

qt_add_qml_module(${PROJECT_NAME}
  URI ${PROJECT_NAME}
  VERSION ${CMAKE_PROJECT_VERSION}
  QML_FILES ${TRACEUI_QML_FILES}
  SOURCES
    backend/SystemMonitor.h
    backend/SystemMonitor.cpp
    backend/ProcessWatcher.h
    backend/ProcessWatcher.cpp
    backend/NetworkMonitor.h
    backend/NetworkMonitor.cpp
    backend/ScrollingHistoryProvider.h
    backend/ScrollingHistoryProvider.cpp
    backend/NetworkHistoryProvider.h
    backend/NetworkHistoryProvider.cpp
    backend/CpuHistoryProvider.h
    backend/CpuHistoryProvider.cpp
    backend/ProcessListModel.h
    backend/ProcessListModel.cpp
)

if(BUILD_TERMINAL)
  add_subdirectory(terminal)
endif()

if(APPLE)
  target_sources(${PROJECT_NAME} PRIVATE
    backend/macos/SystemMonitorMacOS.h
    backend/macos/SystemMonitorMacOS.cpp
    backend/macos/ProcessWatcherMacOS.h
    backend/macos/ProcessWatcherMacOS.cpp
    backend/macos/NetworkMonitorMacOS.h
    backend/macos/NetworkMonitorMacOS.cpp
  )
endif()


if(APPLE)
  target_link_libraries(${PROJECT_NAME} PRIVATE
    "-framework CoreFoundation"
    "-framework IOKit"
  )
endif()

# This is only needed to debug QML in Qt Creator.
# target_compile_definitions(${PROJECT_NAME} PRIVATE
#   "$<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>"
# )

qt6_add_resources(${PROJECT_NAME} "traceui-resources"
  PREFIX "/"
  FILES
    fonts/BlenderPro-Book.ttf
    fonts/Hack-Regular.ttf
    fonts/NF/SymbolsNerdFontMono-Regular.ttf
)

qt6_add_shaders(${PROJECT_NAME} "traceui-shaders"
  BATCHABLE
  PRECOMPILE
  PREFIX "/"
  FILES
    shaders/dotgrid.frag
    shaders/cpugraph.frag
    shaders/memorygrid.frag
    shaders/networkgraph.frag
    shaders/scanlines.frag
)

configure_file(config.h.in config.h)
target_include_directories(${PROJECT_NAME} PRIVATE
  ${CMAKE_CURRENT_BINARY_DIR}  # for config.h
  ${CMAKE_CURRENT_SOURCE_DIR}/backend  # for qml to register C++ types
)

target_link_libraries(${PROJECT_NAME} PRIVATE
  Qt6::Core
  Qt6::Gui
  Qt6::Qml
  Qt6::Quick
  Qt6::QmlModels
  Qt6::QuickControls2
  EncomGlobe
  EncomGlobeplugin
)

if(BUILD_TERMINAL)
  target_link_libraries(${PROJECT_NAME} PRIVATE TraceUITerminal TraceUITerminalplugin)
endif()

# Generate .qmlls.ini
find_program(_qmake NAMES qmake6 qmake HINTS "${Qt6_DIR}/../../../bin")
if(_qmake)
  execute_process(
    COMMAND "${_qmake}" -query QT_INSTALL_QML
    OUTPUT_VARIABLE _qt_qml_dir
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  file(RELATIVE_PATH _qmlls_build_dir "${CMAKE_SOURCE_DIR}" "${CMAKE_BINARY_DIR}")
  file(WRITE "${CMAKE_SOURCE_DIR}/.qmlls.ini"
    "[General]\nbuildDir=${_qmlls_build_dir}\nimportPaths=${_qt_qml_dir}\n"
  )
else()
  message(WARNING "qmake not found. .qmlls.ini will not be generated, which may affect QML language features in some IDEs.")
endif()

# App Bundle
if (WIN32)
  # https://doc.qt.io/qt-6/windows-deployment.html
  set_target_properties(${PROJECT_NAME} PROPERTIES WIN32_EXECUTABLE TRUE)
  find_program(WINDEPLOYQT NAMES windeployqt6 windeployqt HINTS "${Qt6_DIR}/../../../bin" REQUIRED)

  # Add post build command to call windeployqt
  add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
      COMMAND ${WINDEPLOYQT}
      "$<IF:$<CONFIG:Debug>,--debug,--release>"
      --qmldir ${CMAKE_CURRENT_SOURCE_DIR}
      --verbose 1
      "$<TARGET_FILE:${PROJECT_NAME}>"
  )

elseif(APPLE)
  # https://doc.qt.io/qt-6/macos-deployment.html
  # Derive Qt's lib directory from Qt6_DIR (e.g. /opt/homebrew/lib/cmake/Qt6 -> /opt/homebrew/lib)
  cmake_path(GET Qt6_DIR PARENT_PATH _qt_cmake_dir)
  cmake_path(GET _qt_cmake_dir PARENT_PATH _qt_lib_dir)

  set_target_properties(${PROJECT_NAME} PROPERTIES
    MACOSX_BUNDLE TRUE
    MACOSX_BUNDLE_INFO_PLIST cmake/MacOSXBundleInfo.plist.in
    BUILD_RPATH "${_qt_lib_dir}"
  )

  # Find macdeployqt and Qt paths for deployment
  get_target_property(QT_BIN_DIR Qt::qmake IMPORTED_LOCATION)
  find_program(MACDEPLOYQT NAMES macdeployqt HINTS "${QT_BIN_DIR}")

  if(_qmake)
    execute_process(
      COMMAND "${_qmake}" -query QT_INSTALL_PLUGINS
      OUTPUT_VARIABLE _qt_plugins_dir
      OUTPUT_STRIP_TRAILING_WHITESPACE
    )
  endif()

  # Release: run macdeployqt to bundle Qt libraries into the .app and create a .dmg
  # Non-Release: write a qt.conf pointing to the system Qt so the app finds plugins/QML
  add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
      COMMAND ${CMAKE_COMMAND}
          -DMACDEPLOYQT:FILEPATH=${MACDEPLOYQT}
          -DBUNDLE_DIR:PATH=$<TARGET_BUNDLE_DIR:${PROJECT_NAME}>
          -DQMLDIR:PATH=${CMAKE_CURRENT_SOURCE_DIR}
          -DQT_PLUGINS_DIR:PATH=${_qt_plugins_dir}
          -DQT_QML_DIR:PATH=${_qt_qml_dir}
          -DCONFIG:STRING=$<CONFIG>
          -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/RunMacDeployQt.cmake
      COMMENT "Deploying Qt libraries"
      VERBATIM
  )

endif()