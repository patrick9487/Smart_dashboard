cmake_minimum_required(VERSION 3.16)

project(SmartDashboard VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)


find_package(Qt6 REQUIRED COMPONENTS Quick QuickControls2 Qml Gui WaylandCompositor)

qt_standard_project_setup(REQUIRES 6.8)

qt_add_executable(appSmartDashboard
    main.cpp
    AppConfig.cpp
    AppConfig.h
    src/waydroidlauncher.h
    src/waydroidmanager.h
    src/waydroidmanager.cpp
    src/waydroidwindowembedder.h
    src/windowembeditem.h
    src/waylandcompositor.h
    src/surfaceitem.h
    src/xdgshellhelper.h
    src/xdgshellhelper.cpp
)

# 添加資源文件（配置文件）
qt_add_resources(appSmartDashboard "config"
    PREFIX "/"
    FILES
        assets/config.json
)

qt_add_qml_module(appSmartDashboard
    URI SmartDashboard
    VERSION 1.0
    QML_FILES
        qml/DashboardShell.qml
        qml/MainShell.qml
        qml/WidgetFactory.qml
        qml/pages/DashboardDefault.qml
        qml/widgets/TimeWidget.qml
        qml/widgets/TachometerWidget.qml
        qml/widgets/SpeedWidget.qml
        qml/widgets/SpeedLimitWidget.qml
        qml/widgets/FuelGaugeWidget.qml
        qml/widgets/OdometerWidget.qml
        qml/widgets/StatusBarWidget.qml
        qml/widgets/WarningWidget.qml
        qml/widgets/AppIcon.qml
        qml/widgets/AppDock.qml
        qml/widgets/AppWindowEmbed.qml
        qml/widgets/CompositorSurfaceEmbed.qml
    DEPENDENCIES
        QtWayland.Compositor
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(appSmartDashboard PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.appSmartDashboard
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

# 添加 src 目錄到 include 路徑（讓 MOC 能找到 header 文件）
target_include_directories(appSmartDashboard PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(appSmartDashboard
    PRIVATE Qt6::Quick Qt6::QuickControls2 Qt6::Qml Qt6::Gui Qt6::WaylandCompositor
)

include(GNUInstallDirs)
install(TARGETS appSmartDashboard
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# 在構建時，將配置文件複製到構建目錄（作為備用）
# 這樣即使資源系統不工作，也能從文件系統讀取
if(APPLE)
    # 對於 macOS bundle，複製到 Contents/Resources/assets
    add_custom_command(TARGET appSmartDashboard POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory
            $<TARGET_BUNDLE_DIR:appSmartDashboard>/Contents/Resources/assets
        COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_SOURCE_DIR}/assets/config.json
            $<TARGET_BUNDLE_DIR:appSmartDashboard>/Contents/Resources/assets/config.json
        COMMENT "Copying config.json to macOS bundle Resources"
    )
else()
    # 對於其他平台，複製到可執行文件目錄
    add_custom_command(TARGET appSmartDashboard POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory
            $<TARGET_FILE_DIR:appSmartDashboard>/assets
        COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_SOURCE_DIR}/assets/config.json
            $<TARGET_FILE_DIR:appSmartDashboard>/assets/config.json
        COMMENT "Copying config.json to build directory"
    )
endif()
