add_executable(gpgpu)

target_sources(gpgpu PRIVATE
  helpers.cpp
  main.cpp
  orderedMap.cpp
  plugin.cpp
  pluginEngine.cpp
)

target_compile_features(gpgpu PRIVATE cxx_std_14)
target_include_directories(gpgpu PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(gpgpu PRIVATE ${CMAKE_DL_LIBS} project_warnings)
