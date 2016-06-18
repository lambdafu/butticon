cmake_minimum_required(VERSION 3.4)
project(butticon)

include(FindGettext)

include(FindPkgConfig)
find_package(PkgConfig)

add_executable(butticon butticon.c)

set(BUTTICON_NAME butticon)
set(BUTTICON_VERSION_MAJOR 1)
set(BUTTICON_VERSION_MINOR 0)
configure_file(
  "${PROJECT_SOURCE_DIR}/butticon.h.in"
  "${PROJECT_BINARY_DIR}/butticon.h")

include_directories("${PROJECT_BINARY_DIR}")

pkg_check_modules(GLIB REQUIRED glib-2.0)
pkg_check_modules(GTK REQUIRED gtk+-3.0)
target_include_directories(butticon PRIVATE ${GLIB_INCLUDE_DIRS} ${GTK_INCLUDE_DIRS})
target_compile_options(butticon PRIVATE ${GLIB_CFLAGS_OTHER} ${GTK_CFLAGS_OTHER})
target_link_libraries(butticon ${GLIB_LDFLAGS_OTHER} ${GTK_LDFLAGS_OTHER})
target_link_libraries(butticon ${GLIB_LIBRARIES} ${GTK_LIBRARIES})

pkg_check_modules(UPOWER REQUIRED upower-glib)
target_include_directories(butticon PRIVATE ${UPOWER_INCLUDE_DIRS})
target_compile_options(butticon PRIVATE ${UPOWER_CFLAGS_OTHER})
target_link_libraries(butticon ${UPOWER_LDFLAGS_OTHER})
target_link_libraries(butticon ${UPOWER_LIBRARIES})

pkg_check_modules(APPINDICATOR REQUIRED appindicator3-0.1)
target_include_directories(butticon PRIVATE ${APPINDICATOR_INCLUDE_DIRS})
target_compile_options(butticon PRIVATE ${APPINDICATOR_CFLAGS_OTHER})
target_link_libraries(butticon ${APPINDICATOR_LDFLAGS_OTHER})
target_link_libraries(butticon ${APPINDICATOR_LIBRARIES})

install(TARGETS butticon RUNTIME DESTINATION bin)
