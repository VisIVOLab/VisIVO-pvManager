cmake_minimum_required(VERSION 3.19)

project(VisIVO-pvManager VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

message(STATUS "Configuring ${PROJECT_NAME} v${PROJECT_VERSION}")

set(qt_components Core HttpServer)
foreach(module IN LISTS qt_components)
    list(APPEND qt_modules "Qt::${module}")
endforeach()

find_package(Qt6 6.7.0 REQUIRED COMPONENTS ${qt_components})
message(STATUS "Using Qt v${Qt6_VERSION}")

qt_standard_project_setup()

set(PROJECT_SOURCE
    src/main.cpp
    src/Manager.h
    src/Manager.cpp
    src/PVServer.h
    src/PVServer.cpp
)

configure_file(src/version.h.in version.h @ONLY)

qt_add_executable(pvmanager
    ${PROJECT_SOURCE}
)

target_compile_definitions(pvmanager PRIVATE $<$<CONFIG:Release>:QT_NO_DEBUG_OUTPUT>)

target_include_directories(pvmanager PRIVATE
    ${CMAKE_BINARY_DIR}
    "${CMAKE_SOURCE_DIR}/src"
)

target_link_libraries(pvmanager PRIVATE
    ${qt_modules}
)

install(TARGETS pvmanager
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET pvmanager
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)

install(SCRIPT ${deploy_script})

set(CPACK_PACKAGE_VENDOR "Osservatorio Astrofisico di Catania")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "VisIVO Manager for pvserver instances.")
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
set(CPACK_SOURCE_GENERATOR "TGZ")
set(CPACK_SOURCE_IGNORE_FILES
    /CMakeLists.txt.user
    /.git
    /.*build.*
    /\\\\.DS_Store
)
include(CPack)
