set(TEST_SRCS TestRSSpeed.cpp rank9b.cpp bit_array.c poppy.cpp rank9.cpp)

set(FQFEEDER_LIB_SRCS FastxParser.cpp)

add_library(fqfeeder ${FQFEEDER_LIB_SRCS})

include_directories(
${TOP_LEVEL_DIR}/include
../include)

link_directories(${TOP_LEVEL_DIR}/lib 
$ENV{HOME}/lib)

add_executable(test_rank ${TEST_SRCS}) 

target_link_libraries(test_rank pthread fqfeeder z)
