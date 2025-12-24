idf_component_register(SRCS "hardware-test.c" "hardware.c" "test_data_gen.c"
                       INCLUDE_DIRS "."
                       REQUIRES Device_Drivers esp-dsp)
