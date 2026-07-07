idf_component_register(
    SRCS 
        "main.cpp"
    INCLUDE_DIRS "."
    REQUIRES esp_driver_i2c
    REQUIRES esp_timer
    REQUIRES display
)
