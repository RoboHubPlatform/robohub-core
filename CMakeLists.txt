cmake_minimum_required(VERSION 3.16)
project(robohub-core C)

# ---------------------------------------------------------------------------
# Locate robohub-common
# ---------------------------------------------------------------------------
# The runtime is platform-agnostic and reaches hardware only through the common
# contracts, so it links the module that declares them. Built as a dependency
# the generated project adds that module itself; this is for a standalone build.
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/deps.cmake")
  include("${CMAKE_CURRENT_SOURCE_DIR}/deps.cmake")
endif()

if(NOT ROBOHUB_COMMON_DIR)
  if(EXISTS "/home/miraur/projects/robohub-common")
    set(ROBOHUB_COMMON_DIR "/home/miraur/projects/robohub-common")
  elseif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../robohub-common")
    get_filename_component(ROBOHUB_COMMON_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../robohub-common" ABSOLUTE)
  elseif(DEFINED ENV{ROBOHUB_COMMON_DIR})
    set(ROBOHUB_COMMON_DIR "$ENV{ROBOHUB_COMMON_DIR}")
  else()
    message(FATAL_ERROR "robohub-common not found. Set ROBOHUB_COMMON_DIR or place it beside this project.")
  endif()
endif()
message(STATUS "[robohub-core] robohub-common at ${ROBOHUB_COMMON_DIR}")

if(PROJECT_IS_TOP_LEVEL)
  add_subdirectory("${ROBOHUB_COMMON_DIR}" robohub-common)
endif()

# ---------------------------------------------------------------------------
# Library target
# ---------------------------------------------------------------------------
set(ROBOHUB_CORE_SOURCES
    src/core.c
    src/event.c
    src/scheduler.c
    src/app.c
    src/diagnostics.c
)

add_library(robohub-core STATIC ${ROBOHUB_CORE_SOURCES})

target_include_directories(robohub-core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include/robohub/core
    ${ROBOHUB_COMMON_DIR}/include
    ${ROBOHUB_COMMON_DIR}/include/robohub
    ${ROBOHUB_COMMON_DIR}/include/robohub/hal
    ${ROBOHUB_COMMON_DIR}/include/robohub/drivers
)

target_link_libraries(robohub-core PUBLIC robohub-common)
target_compile_features(robohub-core PUBLIC c_std_11)

# ---------------------------------------------------------------------------
# Host tests
# ---------------------------------------------------------------------------
# Ordinary portable C, so they run wherever the build runs. They are white-box:
# the tests include the runtime's private header, so its own source directory is
# on their path.
if(PROJECT_IS_TOP_LEVEL AND RHO_BUILD_HOST_TESTS AND NOT RHO_TARGET_IS_BAREMETAL)
  message(STATUS "[robohub-core] registering host unit tests")
  enable_testing()
  foreach(name event scheduler core)
    add_executable(robohub-core-test-${name} tests/test_${name}.c)
    target_link_libraries(robohub-core-test-${name} PRIVATE robohub-core)
    target_include_directories(robohub-core-test-${name} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src)
    add_test(NAME robohub-core-test-${name} COMMAND robohub-core-test-${name})
  endforeach()
endif()

# ---------------------------------------------------------------------------
# Install
# ---------------------------------------------------------------------------
install(FILES module.yml DESTINATION share/robohub-core)
install(DIRECTORY include/ DESTINATION include)
install(TARGETS robohub-core
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin)
