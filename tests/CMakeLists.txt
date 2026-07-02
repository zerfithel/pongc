set(UNITY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/unity")

if(NOT EXISTS "${UNITY_DIR}/unity.h")
  message(STATUS "Downloading Unity test framework...")
  file(DOWNLOAD 
    "https://raw.githubusercontent.com/ThrowTheSwitch/Unity/master/src/unity.h"
    "${UNITY_DIR}/unity.h"
  )
  file(DOWNLOAD
    "https://raw.githubusercontent.com/ThrowTheSwitch/Unity/master/src/unity.c"
    "${UNITY_DIR}/unity.c"
  )
  file(DOWNLOAD
    "https://raw.githubusercontent.com/ThrowTheSwitch/Unity/master/src/unity_internals.h"
    "${UNITY_DIR}/unity_internals.h"
  )
endif()

add_library(unity STATIC
  "${UNITY_DIR}/unity.c"
)
target_include_directories(unity PUBLIC "${UNITY_DIR}")
target_compile_options(unity PRIVATE -w)

add_executable(pongc_tests
  test_runner.c
  test_utils.c
  
  # Source files under test (excluding main.c)
  ../src/utils.c
)

target_include_directories(pongc_tests PRIVATE
  "${CMAKE_SOURCE_DIR}/src"
  "${UNITY_DIR}"
)

target_link_libraries(pongc_tests PRIVATE
  unity
  m
)

target_compile_features(pongc_tests PRIVATE c_std_23)
target_compile_options(pongc_tests PRIVATE
  -Wall
  -Wextra
  -Wpedantic
)

add_test(NAME unit_tests COMMAND pongc_tests)
