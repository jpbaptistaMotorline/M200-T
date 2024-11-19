idf_component_register(SRCS "rf.c" "wiegand.c" "gpio.c" "core.c" "users.c" "ble_spp_server_demo.c" "rele.c" "inputs.c" "system.c" "sdCard.c" "timer.c" "EG91.c" "ccronexpr.c" "jobs.c" "cron.c" "timegm1.c" "routines.c" "list.c" "pcf85063.c" "crc32.c" "utf8.c" "UDP_Codes.c"  "keeloqDecrypt.c" "inputs.c"
                    INCLUDE_DIRS "."
                    EMBED_TXTFILES "beepSound/som_beep.wav" "beepSound/som_beep_final.wav" "beepSound/alertMotorline.wav" "languages/pt.json" "beepSound/sound_1.wav" "beepSound/sound_2.wav" "beepSound/sound_3.wav" "beepSound/sound_4.wav" "beepSound/sound_5.wav" "beepSound/sound_6.wav" "beepSound/sound_7.wav" "beepSound/sound_8.wav")
                    
                    idf_build_set_property(COMPILE_OPTIONS "-fdiagnostics-color=always" APPEND)
                   
                   




                  
