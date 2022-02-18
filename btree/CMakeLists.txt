add_library(btree src/btree/btree.c include/btree/btree.h)
target_include_directories(btree PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include /usr/local/include)
link_directories("/usr/local/lib")
target_link_libraries(btree PUBLIC "cslice")