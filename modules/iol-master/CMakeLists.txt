zephyr_include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include/iol-master
)

add_subdirectory_ifdef(CONFIG_IOL_MASTER drivers/io_link/cce4511)
add_subdirectory_ifdef(CONFIG_IOL_MASTER subsys/io_link)