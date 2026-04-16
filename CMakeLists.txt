cmake_minimum_required(VERSION 3.22)

#[[
ASE GTK ADAPTER BUILD CONFIGURATION

@file        CMakeLists.txt
@brief       Build configuration for ase-adp-gtk
@description 3rd-party isolation module for gtkmm-4 and libadwaita.
             Wraps Gtk::* / Gio::* / Glib::* / Adw* inheritance and pointer-cast
             patterns behind a clean ase::adp::gtk and ase::adp::adw public API.

@module      ase-adp-gtk
@layer       adapter (3rd-party isolation, consumed by L5 clients)
@created     2026-04-13
@modified    2026-04-13
@version     00.00.01.00001 [seed]
]]

# Parse local VERSION (SSOT) if not building from root
if(NOT DEFINED ASE_ADP_GTK_VERSION)
    if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/VERSION")
        message(FATAL_ERROR "VERSION file not found: ${CMAKE_CURRENT_SOURCE_DIR}/VERSION")
    endif()
    file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/VERSION" _VERSION_LINES)
    foreach(_LINE ${_VERSION_LINES})
        if(_LINE MATCHES "^ASE_ADP_GTK=(.*)$")
            set(ASE_ADP_GTK_VERSION "${CMAKE_MATCH_1}")
        endif()
    endforeach()
    if(NOT DEFINED ASE_ADP_GTK_VERSION)
        message(FATAL_ERROR "ASE_ADP_GTK version not found in VERSION file")
    endif()
endif()

project(ase-adp-gtk VERSION ${ASE_ADP_GTK_VERSION} LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

message(STATUS "Configuring ${PROJECT_NAME} v${PROJECT_VERSION}")

# ── System dependencies ─────────────────────────────────────────────
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTKMM REQUIRED IMPORTED_TARGET gtkmm-4.0)
pkg_check_modules(LIBADW REQUIRED IMPORTED_TARGET libadwaita-1)

# ── Adapter library ─────────────────────────────────────────────────
# Implementations are grouped by domain - one .cpp per logical area.
add_library(ase-adp-gtk STATIC
    src/gtk_widget.cpp        # Widget base + Box, Button, Label, ScrolledWindow, HeaderBar, SearchEntry
    src/gtk_application.cpp   # Application + ApplicationWindow
    src/gtk_list_view.cpp     # ListView + ListItem + ListItemFactory + SingleSelection
    src/gtk_tree.cpp          # TreeListModel + TreeListRow + TreeExpander + DirectoryList + FilterListModel
    src/gtk_menu.cpp          # PopoverMenu + Menu + SimpleActionGroup
    src/gtk_gesture.cpp       # ClickGesture + KeyController + DragSource
    src/gtk_io.cpp            # File + FileMonitor + FileInfo + FileLauncher
    src/gtk_style.cpp         # CssProvider
    src/adw_window.cpp        # Adw Window + ToolbarView + HeaderBar + ViewStack + ViewSwitcher + StyleManager
    src/adw_preferences.cpp   # PreferencesPage + Group + SwitchRow + EntryRow + ComboRow + SpinRow

    # ── Public headers (umbrella) ──
    include/ase/adp/gtk/gtk.hpp
    include/ase/adp/adw/adw.hpp
)
add_library(ase::adp::gtk ALIAS ase-adp-gtk)

target_include_directories(ase-adp-gtk
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_features(ase-adp-gtk PUBLIC cxx_std_20)

target_link_libraries(ase-adp-gtk
    PUBLIC
        PkgConfig::GTKMM
        PkgConfig::LIBADW
)

include(GNUInstallDirs)
install(TARGETS ase-adp-gtk DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
