file(GLOB SRC_FILES "*.c")

find_package(SQLite3 REQUIRED)

add_library(db_sqlite SHARED ${SRC_FILES})
target_link_libraries(db_sqlite PRIVATE SQLite::SQLite3)
