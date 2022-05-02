idf_component_register(
                    SRCS mycamera.c sdcard.c config_json.c http_req.c wifi.c fotolib.c  mygpio.c  myota.c gsm.c
                    INCLUDE_DIRS .
                    PRIV_REQUIRES main nvs_flash app_update
                    )

                    