cmake_minimum_required(VERSION 3.22.2)

project(aluar)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_C_FLAGS_DEBUG "-g3 -fsanitize=address,undefined")
set(CMAKE_CXX_FLAGS_DEBUG "-g3 -fsanitize=address,undefined")

add_compile_options(-Wall -Wextra -Wpedantic -Wconversion)

add_executable(aluar src/main.cpp src/eval.cpp src/lex.cpp src/parse.cpp src/io.cpp src/ast.cpp)
set_property(TARGET aluar PROPERTY CXX_STANDARD 20)
target_include_directories(aluar PRIVATE include/)

# taest testing
#enable_testing()
#add_executable(taest test/test.c)
#target_link_libraries(taest parc)
#set_property(TARGET taest PROPERTY CXX_STANDARD 20)
#
#add_test(NAME Test COMMAND taest)
