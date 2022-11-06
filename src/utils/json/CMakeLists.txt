include_directories(..)
set(JSON_SOURCES
    json_reader.cpp
    json_value.cpp
    json_writer.cpp
)
add_library(JSON ${JSON_SOURCES})