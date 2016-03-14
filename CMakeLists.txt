cmake_minimum_required(VERSION 3.2)

project(asio_prototypes)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package(Boost REQUIRED
  COMPONENTS
    system
    thread)

find_package(TBB REQUIRED)

set(BoostThreadpool_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/threadpool-0_2_5-src/threadpool")

set(Prototypes proto_1 proto_2 proto_3 proto_4 proto_5 proto_6 mergesort)
foreach(prototype ${Prototypes})
  add_executable(${prototype} ${prototype}.cc thread_pool.cc)
  target_include_directories(${prototype}
    PRIVATE SYSTEM
      ${TBB_INCLUDE_DIRS}
      ${Boost_INCLUDE_DIRS}
      ${BoostThreadpool_INCLUDE_DIRS})
  target_link_libraries(${prototype}
    PRIVATE
      ${TBB_LIBRARIES}
      ${Boost_LIBRARIES})
endforeach()
