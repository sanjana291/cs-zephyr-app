zephyr_library_named(iol_master_stack)

zephyr_library_sources(
    iol_mw.c
    iol_dl.c
    iol_main.c
    iol_pl.c
    iol_sm.c
    osal/osal_zephyr.c
)

zephyr_library_include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/osal
)

zephyr_library_link_libraries(iol_master_driver)