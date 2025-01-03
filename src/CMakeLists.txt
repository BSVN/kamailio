# Set the installation paths
message(STATUS "Cross compile: ${CMAKE_CROSSCOMPILING}")

message(STATUS "CMAKE_INSTALL_PREFIX: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "CMAKE_INSTALL_LIBDIR: ${CMAKE_INSTALL_FULL_LIBDIR}")
message(STATUS "CMAKE_INSTALL_BINDIR: ${CMAKE_INSTALL_FULL_BINDIR}")
message(STATUS "CMAKE_INSTALL_SBINDIR: ${CMAKE_INSTALL_FULL_SBINDIR}")
message(STATUS "CMAKE_INSTALL_SYSCONFDIR: ${CMAKE_INSTALL_FULL_SYSCONFDIR}")
message(
  STATUS
    "CMAKE_INSTALL_LOCALSTATEDIR: ${CMAKE_INSTALL_FULL_LOCALSTATEDIR}/run/${MAIN_NAME}"
)
message(STATUS "CMAKE_INSTALL_DATADIR: ${CMAKE_INSTALL_FULL_DATADIR}")
message(STATUS "CMAKE_INSTALL_DOCDIR: ${CMAKE_INSTALL_FULL_DOCDIR}")
message(STATUS "CMAKE_INSTALL_MANDIR: ${CMAKE_INSTALL_FULL_MANDIR}")

message(STATUS "CMAKE_C_COMPILER: ${CMAKE_C_COMPILER}")
message(STATUS "CMAKE_C_COMPILER_ID: ${CMAKE_C_COMPILER_ID}")
message(STATUS "CMAKE_C_COMPILER_VERSION: ${CMAKE_C_COMPILER_VERSION}")

find_package(Threads REQUIRED)

include(${CMAKE_SOURCE_DIR}/cmake/defs.cmake)

# -----------------------
# The following produces core/autover.h Find Git
option(USE_GIT "Use git to get commit information" ON)
find_package(Git QUIET)

if(USE_GIT
   AND GIT_FOUND
   AND EXISTS "${PROJECT_SOURCE_DIR}/.git"
)
  execute_process(
    COMMAND ${GIT_EXECUTABLE} rev-parse --verify --short=6 HEAD
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    OUTPUT_VARIABLE REPO_VER
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )

  execute_process(
    COMMAND
      bash -c
      "${GIT_EXECUTABLE} diff-index --name-only HEAD | grep -vE 'Makefile|CMakeLists.txt'"
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_DIFF_OUTPUT
  )
  set(REPO_HASH ${REPO_VER})

  if(NOT "${GIT_DIFF_OUTPUT}" STREQUAL "")
    set(REPO_VER "${REPO_VER}-dirty")
  endif()

  string(REGEX REPLACE "(.*)-dirty" "dirty" REPO_STATE ${REPO_VER})
else()
  set(REPO_VER "")
  set(REPO_HASH "unknown")
  set(REPO_STATE "")
endif()

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/core/autover.h.in"
  "${CMAKE_CURRENT_SOURCE_DIR}/core/autover.h"
)

# -----------------------
# Policy to enable the Generated property for files required before 3.20.
# This policy allow for not checking if these files exist during the
# configure process since they will be generated when building.
if(${CMAKE_VERSION} VERSION_GREATER_EQUAL 3.20)
  # cmake_policy(VERSION 3.20)
  # https://cmake.org/cmake/help/latest/policy/CMP0118.html#policy:CMP0118
  # CMP0118
  cmake_policy(SET CMP0118 NEW)
else()
  set_source_files_properties(
    ${CMAKE_CURRENT_BINARY_DIR}/core/lex.yy.c
    ${CMAKE_CURRENT_BINARY_DIR}/core/cfg.tab.c
    ${CMAKE_CURRENT_BINARY_DIR}/core/cfg.tab.h PROPERTIES GENERATED TRUE
  )
endif()
# -----------------------

# -----------------------
add_executable(kamailio ${CMAKE_CURRENT_SOURCE_DIR}/main.c)
add_dependencies(kamailio GenerateParser)
# Enable ENABLE_EXPORTS property so modules can link from symbols
# found in kamailio executable.
set_target_properties(
  kamailio PROPERTIES ENABLE_EXPORTS TRUE OUTPUT_NAME ${MAIN_NAME}
)

target_compile_options(kamailio
                       PUBLIC "-ffile-prefix-map=${CMAKE_SOURCE_DIR}/src/=")

add_subdirectory(core)
add_subdirectory(lib)
add_subdirectory(modules)

# These sources are coming from core subdirectory.
# Lib subdirecotry add it sources to kamailio target using target_sources
# from their own CMakeLists.txt
# Module subdiretory does not contribute to the Kamailio executable.
target_sources(
  kamailio
  PRIVATE ${kamailio_SRC_FILES} ${CMAKE_CURRENT_BINARY_DIR}/core/lex.yy.c
          ${CMAKE_CURRENT_BINARY_DIR}/core/cfg.tab.c
)
target_include_directories(kamailio PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/core)

# Debuggng the sources of kamailio executable.
# get_property(
#   kamailio_SRC_PROPERTY
#   TARGET kamailio
#   PROPERTY SOURCES
# )
# message(STATUS "kamailio target sources are: ${kamailio_SRC_PROPERTY}")

target_link_libraries(
  kamailio
  PUBLIC
    common
    m
    Threads::Threads # Pthread library
    # Use of ${CMAKE_DL_LIBS}
    # https://stackoverflow.com/questions/73263834/cmake-how-to-set-rpath-in-a-shared-library-with-only-target-link-directories-w
    ${CMAKE_DL_LIBS} # dl -- dlopen and more
    resolv # resolv
)

# Add the MODS_DIR definition required by main.c ---
target_compile_definitions(
  kamailio PUBLIC MODS_DIR="${CMAKE_INSTALL_FULL_LIBDIR}/${MAIN_NAME}/modules"
)

# Add the install targets Specify the directory on disk to which a file will be
# installed. <dir> should be a relative path. An absolute path is allowed, but
# not recommended. When a relative path is given it is interpreted relative to
# the value of the CMAKE_INSTALL_PREFIX variable.
install(
  TARGETS kamailio
  DESTINATION ${CMAKE_INSTALL_SBINDIR}
  COMPONENT kamailio-core
)

# Install the configuration file (kamailio.cfg) ${CFG_NAME} using a CODE block to check
# existence at install time instead of configure time
# If(EXISTS ..) require full path
install(
  CODE "
    if(EXISTS \"\$ENV{DESTDIR}${CMAKE_INSTALL_FULL_SYSCONFDIR}/${MAIN_NAME}/${CFG_NAME}.cfg\")
        message(STATUS \"${CFG_NAME}.cfg already exists in \$ENV{DESTDIR}${CMAKE_INSTALL_FULL_SYSCONFDIR}/${MAIN_NAME}/${CFG_NAME}.cfg Installing as ${CFG_NAME}.sample\")
      file(INSTALL \"${CMAKE_SOURCE_DIR}/etc/kamailio.cfg\"
        DESTINATION \"${CMAKE_INSTALL_FULL_SYSCONFDIR}/${MAIN_NAME}\"
        RENAME \"${CFG_NAME}.cfg.sample\"
        PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
      )
    else()
        file(INSTALL \"${CMAKE_SOURCE_DIR}/etc/kamailio.cfg\"
          DESTINATION \"${CMAKE_INSTALL_FULL_SYSCONFDIR}/${MAIN_NAME}\"
          PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
          RENAME \"${CFG_NAME}.cfg\"
        )
    endif()

"
  COMPONENT kamailio-utils-cfg
)
