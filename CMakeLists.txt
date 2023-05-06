cmake_minimum_required(VERSION 3.10.2)
project(zsh_history)
enable_language(C)

# add_compile_options(-W -Wall -Werror -std=c17 -pedantic)
add_compile_options(-W -Wall -std=c99 -pedantic)
# add_compile_options(-W -Wall -std=c17 -pedantic)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0")

include_directories(${CURRENT_SOURCE_DIR})

# sqlite3 via package manager
find_package(SQLite3)
include_directories(${SQLite3_INCLUDE_DIRS})

# add_executable(${PROJECT_NAME} main.c) add_executable(db db.c)
# add_executable(cli main.c)
add_executable(cli test.c str.c stack.c)

# target_link_libraries(db dl m pthread ${SQLite3_LIBRARIES})
target_link_libraries(cli dl m pthread ${SQLite3_LIBRARIES})

# install(TARGETS ${PROJECT_NAME} DESTINATION bin) install(TARGETS db
# DESTINATION bin)
install(TARGETS cli DESTINATION bin)
