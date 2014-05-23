cmake_minimum_required(VERSION 2.6)

project(desd-controller)

find_package(Boost 1.49 REQUIRED
             COMPONENTS date_time program_options system
            )
include_directories(SYSTEM ${Boost_INCLUDE_DIRS})

# Use -DCMAKE_BUILD_TYPE to control optimization and debugging
set(CMAKE_CXX_FLAGS
    "-pedantic -std=c++98 -Wall -Wextra -pthread ${CMAKE_CXX_FLAGS}")

add_executable(desd-controller desd-interface.cpp
                               desd-interface.hpp
                               dgi-interface.cpp
                               dgi-interface.hpp
                               io-interface.hpp
                               main.cpp
              )

target_link_libraries(desd-controller ${Boost_LIBRARIES})
