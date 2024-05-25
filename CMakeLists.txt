cmake_minimum_required(VERSION 3.27)
project(chk_lib_win C)

add_subdirectory("external")

set(src "win.c")
set(hdr "win.h")

list(TRANSFORM src PREPEND "src/")
list(TRANSFORM hdr PREPEND "include/chk/win/")

add_library(chk_win ${src} ${hdr})
add_library(chk::win ALIAS chk_win)

target_compile_features(chk_win PUBLIC c_std_11)
target_include_directories(chk_win PUBLIC "include")

target_link_libraries(chk_win PUBLIC chk::core)
target_link_libraries(chk_win PRIVATE glfw::glfw optimized glad::gl debug glad::gl_d)
