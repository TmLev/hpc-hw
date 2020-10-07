function(enable_thread_sanitizer PROJECT_OPTIONS)
  target_compile_options(${PROJECT_OPTIONS} INTERFACE -fsanitize=thread)
  target_link_libraries(${PROJECT_OPTIONS}  INTERFACE -fsanitize=thread)
endfunction()
