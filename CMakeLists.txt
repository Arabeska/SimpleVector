cmake_minimum_required(VERSION 3.11)

project(VectorSimple CXX)
set(CMAKE_CXX_STANDARD 17)

set(VECTOR_FILES array_ptr.h simple_vector.h)

if(CMAKE_SYSTEM_NAME MATCHES "^MINGW")
    set(SYSTEM_LIBS -lstdc++)
else()
    set(SYSTEM_LIBS)
endif()

add_executable(simpl_vector main.cpp ${VECTOR_FILES} tests.h)

target_link_libraries(simpl_vector ${SYSTEM_LIBS})