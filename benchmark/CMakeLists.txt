include(FetchContent)

FetchContent_Declare(
  googlebenchmark
  GIT_REPOSITORY https://github.com/google/benchmark.git
  GIT_TAG v1.9.1)

set(BENCHMARK_ENABLE_TESTING OFF)
FetchContent_MakeAvailable(googlebenchmark)

function(lob_add_perf_bench impl_name source)
  add_executable(${PROJECT_NAME}_perf_${impl_name} ${source})

  target_link_libraries(${PROJECT_NAME}_perf_${impl_name}
                        PRIVATE ${PROJECT_NAME})

  target_include_directories(${PROJECT_NAME}_perf_${impl_name}
                             PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

  target_compile_options(${PROJECT_NAME}_perf_${impl_name}
                         PRIVATE -Wall -Wextra -Wpedantic)
endfunction()
