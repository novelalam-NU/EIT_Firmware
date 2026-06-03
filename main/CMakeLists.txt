idf_component_register(SRCS "main.c" "../Application_Layer/wireless.c" "../Application_Layer/calibration.c" "../Application_Layer/measurement.c" "../Application_Layer/tasks.c" "../Application_Layer/inference.c" "../Application_Layer/test.c"
                    INCLUDE_DIRS "." "../Application_Layer" "../Middle_Ware"
                    REQUIRES Middle_Ware driver esp_wifi esp_event nvs_flash esp_netif lwip)
