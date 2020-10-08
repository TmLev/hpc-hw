add_library(await INTERFACE)
target_include_directories(await INTERFACE include)
target_link_libraries(
  await
  INTERFACE project_warnings
            project_options
            pthread
)
