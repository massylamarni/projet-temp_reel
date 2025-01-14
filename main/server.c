#include "server.h"
#include "esp_system.h"
#include <stdio.h>
httpd_handle_t server = NULL; // Handler of the server 
// External file inclusions for resources
extern const uint8_t open_png_start[] asm("_binary_open_png_start");
extern const uint8_t open_png_end[] asm("_binary_open_png_end");
extern const uint8_t closed_png_start[] asm("_binary_closed_png_start");
extern const uint8_t closed_png_end[] asm("_binary_closed_png_end");
extern const uint8_t index_html_start[] asm("_binary_index_html_start");
extern const uint8_t index_html_end[] asm("_binary_index_html_end");
extern const uint8_t app_js_start[] asm("_binary_app_js_start");
extern const uint8_t app_js_end[] asm("_binary_app_js_end");
extern const uint8_t app_js_end[] asm("_binary_app_js_end");
extern const uint8_t app_css_start[] asm("_binary_app_css_start");
extern const uint8_t app_css_end[] asm("_binary_app_css_end");
extern const uint8_t favicon_ico_start[] asm("_binary_favicon_ico_start");
extern const uint8_t favicon_ico_end[] asm("_binary_favicon_ico_end");
// Handler pour l'image open.png
static esp_err_t http_server_open_png_handler(httpd_req_t *req) {
    httpd_resp_set_type(req, "image/png");
    httpd_resp_send(req, (const char*)open_png_start, open_png_end - open_png_start);
    return ESP_OK;
}

// Handler pour l'image closed.png
static esp_err_t http_server_closed_png_handler(httpd_req_t *req) {
    httpd_resp_set_type(req, "image/png");
    httpd_resp_send(req, (const char*)closed_png_start, closed_png_end - closed_png_start);
    return ESP_OK;
}

// HTML handler
static esp_err_t http_server_html_handler(httpd_req_t *req) {
    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, (const char*)index_html_start, index_html_end - index_html_start);
    return ESP_OK;
}

// CSS handler
static esp_err_t http_server_app_css_handler(httpd_req_t *req) {
    httpd_resp_set_type(req, "text/css");
    httpd_resp_send(req, (const char*)app_css_start,app_css_end - app_css_start);
    return ESP_OK;
}

// JavaScript handler
static esp_err_t http_server_app_js_handler(httpd_req_t *req) {
    httpd_resp_set_type(req, "application/javascript");
    httpd_resp_send(req, (const char*)app_js_start, app_js_end - app_js_start);
    return ESP_OK;
}

// Favicon handler
static esp_err_t http_server_favicon_ico_handler(httpd_req_t *req) {
    httpd_resp_set_type(req, "image/x-icon");
    httpd_resp_send(req, (const char*)favicon_ico_start, favicon_ico_end - favicon_ico_start);
    return ESP_OK;
}
static esp_err_t get_temperature_handler(httpd_req_t *req) {
    // Créer une structure JSON pour envoyer la température, l'humidité et l'état de la porte
    char response[256];
    
    // Ces valeurs doivent provenir de tes capteurs ou variables
    float temperature =  get_temperature();  //lire la température 
    float humidity = 10;     //  l'humidité actuelle
    float gas = Detecteur_gaz();         // capteur de gaz
    bool doorState = false;

    // Formatage de la réponse JSON avec les données actuelles
    snprintf(response, sizeof(response), "{\"temperature\": %.1f, \"humidity\": %.1f, \"doorState\": %s, \"gas\": %.1f}",
             temperature, humidity, doorState ? "true" : "false", gas);

    // Définir le type de la réponse HTTP
    httpd_resp_set_type(req, "application/json");
    
    // Envoyer la réponse au client
    httpd_resp_send(req, response, strlen(response));
    
    return ESP_OK;
}
esp_err_t http_config_and_start(void) {
    // HTTP server configuration
    httpd_config_t server_config = HTTPD_DEFAULT_CONFIG();
    server_config.task_priority = 7; 
    esp_err_t res = httpd_start(&server, &server_config);
    if (res != ESP_OK) {
        printf("Error starting HTTP server\n");
    } else {
        printf("HTTP server started\n");
    // Register handlers
httpd_uri_t index_html = {
    .uri = "/",
    .method = HTTP_GET,
    .handler = http_server_html_handler,
    .user_ctx = NULL
};

httpd_uri_t app_css = {
    .uri = "/app.css",
    .method = HTTP_GET,
    .handler = http_server_app_css_handler,
    .user_ctx = NULL
};

httpd_uri_t app_js = {
    .uri = "/app.js",
    .method = HTTP_GET,
    .handler = http_server_app_js_handler,
    .user_ctx = NULL
};

httpd_uri_t favicon_ico = {
    .uri = "/favicon.ico",
    .method = HTTP_GET,
    .handler = http_server_favicon_ico_handler,
    .user_ctx = NULL
};
httpd_uri_t open_png = {
    .uri = "/open.png",
    .method = HTTP_GET,
    .handler = http_server_open_png_handler,
    .user_ctx = NULL
};

httpd_uri_t closed_png = {
    .uri = "/closed.png",
    .method = HTTP_GET,
    .handler = http_server_closed_png_handler,
    .user_ctx = NULL
};
httpd_uri_t temperature_uri = {
    .uri = "/temperature",        // L'URL à laquelle le client fera une requête GET
    .method = HTTP_GET,           // La méthode HTTP (GET dans ce cas)
    .handler = get_temperature_handler,  // La fonction qui gère cette route
    .user_ctx = NULL
};


// Enregistrement des handlers pour les images
   ESP_ERROR_CHECK(httpd_register_uri_handler(server, &temperature_uri));
   ESP_ERROR_CHECK(httpd_register_uri_handler(server, &open_png)); 
   ESP_ERROR_CHECK(httpd_register_uri_handler(server, &closed_png));
   ESP_ERROR_CHECK(httpd_register_uri_handler(server, &index_html));
   ESP_ERROR_CHECK(httpd_register_uri_handler(server, &app_css));
   ESP_ERROR_CHECK(httpd_register_uri_handler(server, &app_js));
   ESP_ERROR_CHECK(httpd_register_uri_handler(server, &favicon_ico));
    }
    return res;
}
