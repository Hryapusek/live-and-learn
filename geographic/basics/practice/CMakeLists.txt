cmake_minimum_required(VERSION 3.16)

project(geo-basics)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

function(setup_simple_practice func_name)
add_executable(${func_name})

target_sources(${func_name}
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/${func_name}.cpp
)
endfunction()

setup_simple_practice(practice1)
setup_simple_practice(practice2)
setup_simple_practice(practice3)
setup_simple_practice(practice4)

setup_simple_practice(practice5)
target_link_libraries(practice5 PRIVATE Geographic)
