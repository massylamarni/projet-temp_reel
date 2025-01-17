#ifndef SERVER_H
#define SERVER_H
#include "stdbool.h"
#include <esp_http_server.h>
#include <esp_err.h>
#include "capture_temp.h"
#include "capture_gaz.h"
// Déclaration des ressources externes
extern const uint8_t open_png_start[];
extern const uint8_t open_png_end[];
extern const uint8_t closed_png_start[];
extern const uint8_t closed_png_end[];
extern const uint8_t index_html_start[];
extern const uint8_t index_html_end[];
extern const uint8_t app_js_start[];
extern const uint8_t app_js_end[];
extern const uint8_t app_css_start[];
extern const uint8_t app_css_end[];
extern const uint8_t favicon_ico_start[];
extern const uint8_t favicon_ico_end[];

// Prototypes des handlers 
static esp_err_t http_server_open_png_handler(httpd_req_t *req);
static esp_err_t http_server_closed_png_handler(httpd_req_t *req);
static esp_err_t http_server_html_handler(httpd_req_t *req);
static esp_err_t http_server_app_css_handler(httpd_req_t *req);
static esp_err_t http_server_app_js_handler(httpd_req_t *req);
static esp_err_t http_server_favicon_ico_handler(httpd_req_t *req);
static esp_err_t get_temperature_handler(httpd_req_t *req);

// Déclaration publique de la fonction pour configurer le serveur HTTP
esp_err_t http_config_and_start(void);

#endif // SERVER_H
