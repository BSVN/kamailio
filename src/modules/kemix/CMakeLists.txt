file(GLOB SRC_FILES "*.c")

add_library(kemix SHARED ${SRC_FILES})
# target_link_libraries(.*PRIVATE)

target_compile_options(
  kemix PUBLIC "-ffile-prefix-map=${CMAKE_SOURCE_DIR}/src/modules/kemix/=")
set_target_properties(kemix PROPERTIES SUFFIX ".so")
