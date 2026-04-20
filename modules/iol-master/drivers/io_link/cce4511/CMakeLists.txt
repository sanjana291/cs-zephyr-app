zephyr_library_named(iol_master_driver)

zephyr_library_sources(
    iol_cce4511_pl.c
    osal/osal_spi.c
    osal/osal_irq_stub.c
)

zephyr_library_include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/osal
)