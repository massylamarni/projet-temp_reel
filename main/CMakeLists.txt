idf_component_register(SRCS "capture_rfid.c" "main.c" "idir_wifi.c" "server.c" "capture_gaz.c" "capture_temp.c"
                    INCLUDE_DIRS "."
                    EMBED_FILES webpage/app.css webpage/app.js webpage/favicon.ico webpage/index.html webpage/open.png webpage/closed.png)
