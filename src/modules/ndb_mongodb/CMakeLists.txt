find_package(PkgConfig REQUIRED)
pkg_check_modules(mongoc REQUIRED IMPORTED_TARGET libmongoc-1.0>=1.7)
add_library(mongoc::mongoc ALIAS PkgConfig::mongoc)

file(GLOB SRC_FILES "*.c")

add_library(ndb_mongodb SHARED ${SRC_FILES})
target_link_libraries(ndb_mongodb PRIVATE mongoc::mongoc)

target_compile_options(
  ndb_mongodb
  PUBLIC "-ffile-prefix-map=${CMAKE_SOURCE_DIR}/src/modules/ndb_mongodb/=")
set_target_properties(ndb_mongodb PROPERTIES SUFFIX ".so")
