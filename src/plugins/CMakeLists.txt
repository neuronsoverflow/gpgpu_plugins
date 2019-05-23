# hello.so
add_library(hello SHARED)
target_sources(hello PRIVATE hello.c)
set_target_properties(hello PROPERTIES PREFIX "")
target_compile_features(hello PRIVATE cxx_std_14)
target_link_libraries(hello PRIVATE ${CMAKE_DL_LIBS} project_warnings)
target_include_directories(hello PRIVATE ${PROJECT_SOURCE_DIR}/include include)

# prime.so
add_library(prime SHARED)
target_sources(prime PRIVATE prime.cpp prime.cu)
set_target_properties(prime PROPERTIES PREFIX "")
target_compile_features(prime PRIVATE cxx_std_14)
# target_link_libraries(prime PRIVATE ${CMAKE_DL_LIBS} project_warnings)
target_include_directories(prime PRIVATE ${PROJECT_SOURCE_DIR}/include include)

# matrix.so
add_library(matrix SHARED)
target_sources(matrix PRIVATE matrix.cpp matrix.cu)
set_target_properties(matrix PROPERTIES PREFIX "")
target_compile_features(matrix PRIVATE cxx_std_14)
# target_link_libraries(matrix PRIVATE ${CMAKE_DL_LIBS} project_warnings)
target_include_directories(matrix PRIVATE ${PROJECT_SOURCE_DIR}/include include)

# graph.so
add_library(graph SHARED)
target_sources(graph PRIVATE graph.cpp graph.cu)
set_target_properties(graph PROPERTIES PREFIX "")
target_compile_features(graph PRIVATE cxx_std_14)
# target_link_libraries(graph PRIVATE ${CMAKE_DL_LIBS} project_warnings)
target_include_directories(graph PRIVATE ${PROJECT_SOURCE_DIR}/include include)
