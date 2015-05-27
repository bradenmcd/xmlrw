cmake_minimum_required(VERSION 3.0)
cmake_policy(SET CMP0048 NEW)

project(
    xmlrw
    VERSION 0.1.0
    LANGUAGES CXX
)

set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
set(CPACK_SOURCE_PACKAGE_FILE_NAME "${PROJECT_NAME}-${PROJECT_VERSION}")
set(CPACK_SOURCE_IGNORE_FILES "/\\\\.git/" "\\\\.#" "/#" ".*~$")
include(CPack)

set(CMAKE_CXX_STANDARD 11)

option(BUILD_USE_STATIC_RUNTIME "Use the static runtime library on Windows"
       OFF)
option(BUILD_WITH_XMLLITE "Use XmlLite (Windows XP SP2+)" OFF)

if(NOT BUILD_WITH_XMLLITE)
    set(REQUIRE_LIBXML2 REQUIRED)
endif()

find_package(LibXml2 ${REQUIRE_LIBXML2})
find_package(Doxygen)
find_package(Perl)

if(BUILD_USE_STATIC_RUNTIME)
    foreach(LANG_TAG C CXX)
        foreach (CONFIGURATION "" _DEBUG _RELEASE _MINSIZEREL _RELWITHDEBINFO)
            if(CMAKE_${LANG_TAG}_FLAGS${CONFIGURATION} MATCHES "/MD")
                string(REGEX REPLACE "/MD" "/MT" CMAKE_${LANG_TAG}_FLAGS${CONFIGURATION} "${CMAKE_${LANG_TAG}_FLAGS${CONFIGURATION}}")
            endif()
        endforeach()
    endforeach()
endif()

if(WIN32)
    set(XMLRW_WIN32_WINNT 0x500 CACHE STRING "Minimum supported Windows version")
    add_definitions(
        -D _WIN32_WINNT=${XMLRW_WIN32_WINNT}
        -D _UNICODE
        -D NOMINMAX
    )
endif()

add_subdirectory(src)

configure_file(Doxyfile.in Doxyfile @ONLY)
