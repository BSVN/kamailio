file(GLOB SRC_FILES "*.c")

add_library(dialog SHARED ${SRC_FILES})

# target_link_libraries(.*PRIVATE)

target_compile_options(
  dialog PUBLIC "-ffile-prefix-map=${CMAKE_SOURCE_DIR}/src/modules/dialog/=")
set_target_properties(dialog PROPERTIES SUFFIX ".so")
