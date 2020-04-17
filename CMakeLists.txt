cmake_minimum_required(VERSION 3.12)
project(miranda CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# add compiler warnings
add_compile_options(-Wall -Wextra -Wpedantic)

# -------------------- SST CONFIGS --------------------
execute_process(
        COMMAND which sst-config
        OUTPUT_VARIABLE SST_CONFIG
        OUTPUT_STRIP_TRAILING_WHITESPACE
)
execute_process(
        COMMAND ${SST_CONFIG} --ELEMENT_CXXFLAGS
        OUTPUT_VARIABLE SST_FLAGS
        OUTPUT_STRIP_TRAILING_WHITESPACE
)
execute_process(
        COMMAND ${SST_CONFIG} --ELEMENT_LDFLAGS
        OUTPUT_VARIABLE SST_LDFLAGS
        OUTPUT_STRIP_TRAILING_WHITESPACE
)
separate_arguments(SST_FLAGS)
separate_arguments(SST_LDFLAGS)

macro(_sst_compile_link _name)
    if (TARGET ${_name})
        target_compile_definitions(${_name} PRIVATE)
        target_compile_options(${_name} PRIVATE ${SST_FLAGS})
        target_link_options(${_name} PRIVATE ${SST_LDFLAGS})
    endif ()
endmacro()
# -------------------- SST CONFIGS --------------------

# add compiler warnings
add_compile_options(-Wall -Wextra -Wpedantic)

# -------------------- SST EXECUTABLES --------------------
add_executable(
        libmiranda.so
        mirandaCPU.cc
        mirandaIncGen.cc
        generators/stencil3dbench.cc
        generators/streambench.cc
        generators/revsinglestream.cc
        generators/streambench_customcmd.cc
        generators/gupsgen.cc
        generators/singlestream.cc
        generators/randomgen.cc
)
_sst_compile_link(libmiranda.so)
# -------------------- SST EXECUTABLES --------------------
