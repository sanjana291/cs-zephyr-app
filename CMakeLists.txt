# Calixto platform module — root CMakeLists.txt
# Each driver module under modules/ is included independently.
# To add a new driver: add_subdirectory(modules/<your-driver>)

add_subdirectory(modules/iol-master)

# Future drivers — uncomment as you add them:
# add_subdirectory(modules/calixto-ethernet)
# add_subdirectory(modules/calixto-eeprom)
# add_subdirectory(modules/calixto-wifi)
