set(SOURCES
        "PlantMonitoringSensorAio.cpp"
        "wifiManager/wifiManager.cpp"
)

set(INCLUDE_DIRS
        ""
        "wifiManager"
)

idf_component_register(SRCS ${SOURCES}
        INCLUDE_DIRS ${INCLUDE_DIRS})
