set(SOURCES
        "PlantMonitoringSensorAio.cpp"
        "wifiManager/wifiManager.cpp"
        "mqttManager/mqttManager.cpp"
)

set(INCLUDE_DIRS
        ""
        "wifiManager"
        "mqttManager"
)

idf_component_register(SRCS ${SOURCES}
        INCLUDE_DIRS ${INCLUDE_DIRS})
