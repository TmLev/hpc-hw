function(set_project_warnings PROJECT_WARNINGS)
  target_compile_options(
    ${PROJECT_WARNINGS}
    INTERFACE -Wall
              -Wextra
              -Wshadow
              -Wnon-virtual-dtor
              -Wold-style-cast
              -Wcast-align
              -Wunused
              -Woverloaded-virtual
              -Wpedantic
              -Wconversion
              -Wsign-conversion
              -Wnull-dereference
              -Wdouble-promotion
              -Wformat=2
  )
endfunction()
