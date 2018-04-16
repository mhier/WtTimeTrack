cmake_minimum_required(VERSION 2.6)
project(WtTimeTrack)

set(WT_CONNECTOR "Wt::HTTP" CACHE STRING "Connector used (Wt::HTTP or Wt::FCGI)")

set(WtTimeTrack_TARGET wt_project.wt)

find_package(Boost REQUIRED COMPONENTS program_options signals system filesystem thread)
find_package(wt REQUIRED COMPONENTS http fcgi dbo dbosqlite3 dbomysql)

add_custom_target(link_target ALL
                  COMMAND ${CMAKE_COMMAND} -E create_symlink /usr/share/Wt/resources ${CMAKE_BINARY_DIR}/resources)

configure_file(${CMAKE_SOURCE_DIR}/wt_project ${CMAKE_BINARY_DIR}/wt_project COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/WtTimeTrack.css ${CMAKE_BINARY_DIR}/WtTimeTrack.css COPYONLY)

aux_source_directory(${CMAKE_SOURCE_DIR}/src WtTimeTrack_SOURCE)
include_directories(${CMAKE_SOURCE_DIR}/include)

add_executable(${WtTimeTrack_TARGET} ${WtTimeTrack_SOURCE})
target_link_libraries(${WtTimeTrack_TARGET} ${WT_CONNECTOR} Wt::DboMySQL Wt::DboSqlite3 ${Boost_LIBRARIES})
