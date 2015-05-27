include(GNUInstallDirs)

if(NOT BUILD_WITH_XMLLITE)
    include_directories(${LIBXML2_INCLUDE_DIR})
endif()

set(HEADERS
    xml/reader.h
    xml/writer.h
)

set(SOURCES
    xml/finally.h
    xml/reader.cpp
    xml/writer.cpp
)

if(BUILD_WITH_XMLLITE)
    list(APPEND SOURCES
        xml/stringconvert.h
        xml/stringconvert.cpp
        xml/xmllite_errmsg.h
        xml/xmllite_errmsg.cpp
    )
endif()

add_library(xmlrw STATIC ${HEADERS} ${SOURCES})

if(BUILD_WITH_XMLLITE)
    target_compile_definitions(xmlrw PRIVATE HAVE_XMLLITE)
    target_link_libraries(xmlrw PRIVATE XmlLite Shlwapi)
else()
    target_link_libraries(xmlrw PRIVATE ${LIBXML2_LIBRARIES})
endif()

install(
    TARGETS xmlrw
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(
    FILES ${HEADERS}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/xml
)
