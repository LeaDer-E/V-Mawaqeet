cmake_minimum_required(VERSION 3.16)
project(V-Mawaqeet VERSION 2.6.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Network)
find_package(Qt6 QUIET COMPONENTS Multimedia Positioning)

set(SOURCES
    src/main.cpp
    src/paths.h
    src/paths.cpp
    src/config.h
    src/config.cpp
    src/autostart.h
    src/autostart.cpp
    src/hijri.h
    src/hijri.cpp
    src/methods.h
    src/methods.cpp
    src/prayer_calc.h
    src/prayer_calc.cpp
    src/locations.h
    src/locations.cpp
    src/geo.h
    src/geo.cpp
    src/providers.h
    src/providers.cpp
    src/i18n.h
    src/i18n.cpp
    src/theme.h
    src/theme.cpp
    src/workers.h
    src/state.h
    src/state.cpp
    src/notifier.h
    src/notifier.cpp
    src/sound.h
    src/sound.cpp
    src/ui/icons.h
    src/ui/icons.cpp
    src/ui/art.h
    src/ui/art.cpp
    src/ui/widgets.h
    src/ui/widgets.cpp
    src/ui/locate.h
    src/ui/locate.cpp
    src/ui/notification.h
    src/ui/notification.cpp
    src/ui/about_dialog.h
    src/ui/about_dialog.cpp
    src/ui/home.h
    src/ui/home.cpp
    src/ui/themes_page.h
    src/ui/themes_page.cpp
    src/ui/settings_page.h
    src/ui/settings_page.cpp
    src/ui/onboarding.h
    src/ui/onboarding.cpp
    src/ui/window.h
    src/ui/window.cpp
)

if(WIN32)
    list(APPEND SOURCES app.rc)
    add_executable(V-Mawaqeet WIN32 ${SOURCES})
    target_link_libraries(V-Mawaqeet PRIVATE shell32 ole32)
else()
    add_executable(V-Mawaqeet ${SOURCES})
endif()

target_include_directories(V-Mawaqeet PRIVATE src src/ui)

target_link_libraries(V-Mawaqeet PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
)

if(Qt6Multimedia_FOUND)
    target_compile_definitions(V-Mawaqeet PRIVATE VM_HAS_MULTIMEDIA=1)
    target_link_libraries(V-Mawaqeet PRIVATE Qt6::Multimedia)
endif()

if(Qt6Positioning_FOUND)
    target_compile_definitions(V-Mawaqeet PRIVATE VM_HAS_POSITIONING=1)
    target_link_libraries(V-Mawaqeet PRIVATE Qt6::Positioning)
endif()

# Copy assets folder next to the compiled executable automatically after build
add_custom_command(TARGET V-Mawaqeet POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/assets"
        "$<TARGET_FILE_DIR:V-Mawaqeet>/assets"
    COMMENT "Copying assets/ next to V-Mawaqeet binary..."
)

install(TARGETS V-Mawaqeet RUNTIME DESTINATION bin)
install(DIRECTORY assets DESTINATION share/V-Mawaqeet)
