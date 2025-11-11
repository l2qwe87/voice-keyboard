idf_component_register(SRCS "main.c"
                            "config/i2s_config.c"
                            "config/gpio_config.c"
                    INCLUDE_DIRS "."
                    REQUIRES driver esp_common)
