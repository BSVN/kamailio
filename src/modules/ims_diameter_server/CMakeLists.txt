file(GLOB SRC_FILES "*.c")

add_library(ims_diameter_server SHARED ${SRC_FILES})

# target_link_libraries(.*PRIVATE)
