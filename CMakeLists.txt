cmake_minimum_required(VERSION 3.20)

# FORCE both C and C++ compilers to load
project(kb_search VERSION 1.0 LANGUAGES C CXX)
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
set(CMAKE_CXX_STANDARD 17)

# --- wxWidgets ---
set(wxWidgets_ROOT_DIR "C:/wxWidgets-3.2.2/msvc-install")
find_package(wxWidgets COMPONENTS core base REQUIRED)
include(${wxWidgets_USE_FILE})

# --- Executable Configuration ---
# We are listing every file manually with its path relative to CMakeLists.txt
add_executable(kb_search
    third_party/sqlite3/sqlite3.c
    src/Database.cpp
    src/EntryCard.cpp
    src/main.cpp
    src/MainFrame.cpp
    src/ResultsPanel.cpp
    src/SearchEngine.cpp
    src/TopicPanel.cpp
)
# Add these lines after add_executable
set_source_files_properties(third_party/sqlite3/sqlite3.c PROPERTIES 
    COMPILE_DEFINITIONS "SQLITE_ENABLE_FTS5;SQLITE_ENABLE_FTS4;SQLITE_ENABLE_RTREE;SQLITE_ENABLE_COLUMN_METADATA"
)

# Also apply it to the whole project just to be safe
add_definitions(-DSQLITE_ENABLE_FTS5)
# Tell MSVC: "Even though this is a C++ project, compile THIS file as C"
set_source_files_properties(third_party/sqlite3/sqlite3.c PROPERTIES LANGUAGE C)

target_include_directories(kb_search PRIVATE 
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/third_party/sqlite3
    ${wxWidgets_INCLUDE_DIRS}
)

target_link_libraries(kb_search PRIVATE ${wxWidgets_LIBRARIES})

# This fixes the "unresolved external symbol main"
set_target_properties(kb_search PROPERTIES WIN32_EXECUTABLE TRUE)