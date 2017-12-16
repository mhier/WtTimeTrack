CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(WTtest)

SET(WT_CONNECTOR "wthttp" CACHE STRING "Connector used (wthttp or wtfcgi)")

aux_source_directory(${CMAKE_SOURCE_DIR}/src WTtest_SOURCE)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/include)

SET(WTtest_TARGET wt_project.wt)

INCLUDE_DIRECTORIES(/home/mhier/compile/wt-4.0.2/install/include)
LINK_DIRECTORIES(/home/mhier/compile/wt-4.0.2/install/lib)

ADD_CUSTOM_TARGET(link_target ALL
                  COMMAND ${CMAKE_COMMAND} -E create_symlink /usr/share/Wt/resources ${CMAKE_BINARY_DIR}/resources)

CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/wt_project ${CMAKE_BINARY_DIR}/wt_project COPYONLY)
CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/zeiterfassung.css ${CMAKE_BINARY_DIR}/zeiterfassung.css COPYONLY)

ADD_EXECUTABLE(${WTtest_TARGET} ${WTtest_SOURCE})
TARGET_LINK_LIBRARIES(${WTtest_TARGET} ${WT_CONNECTOR} wt wtdbo wtdbosqlite3 boost_system boost_signals crypt)
