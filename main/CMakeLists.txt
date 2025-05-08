# Set source files


set(CONFIG_EXAMPLE_AVOID_TEAR_EFFECT_WITH_SEM OFF CACHE BOOL "tear")

set(C_SOURCES
    "EXIO/TCA9554PWR.c"
    "LCD_Driver/ST7701S.c"
    "Touch_Driver/GT911.c"
    "Touch_Driver/esp_lcd_touch/esp_lcd_touch.c"
    "I2C_Driver/I2C_Driver.c"
    "PCF85063/PCF85063.c"
    "QMI8658/QMI8658.c"
    "SD_Card/SD_MMC.c"
    "Wireless/Wireless.c"
    "Buzzer/Buzzer.c"
    "BAT_Driver/BAT_Driver.c"
)

set(CPP_SOURCES
    "main.cpp"
)

# Register component
idf_component_register(
    SRCS ${C_SOURCES} ${CPP_SOURCES}
    INCLUDE_DIRS
        "./EXIO"
        "./LCD_Driver"
        "./Touch_Driver"
        "./Touch_Driver/esp_lcd_touch"
        "./I2C_Driver"
        "./PCF85063"
        "./QMI8658"
        "./SD_Card"
        "./Wireless"
        "./Buzzer"
        "./BAT_Driver"
        "./2DSofwareRenderer/SoftRendererLib/src/include"
        "."
)


target_link_libraries(${COMPONENT_LIB} PRIVATE SoftRendererLib)
