idf_component_register(SRCS "calibration.c"
                            "inference.c"
                            "measurement.c"
                            "tasks.c"
                            "test.c"
                            "wireless.c"
                       INCLUDE_DIRS "."
                       REQUIRES Middle_Ware Device_Drivers espressif__esp-dsp)
