/*
 * http_server.c
 *
 *  Created on: Oct 20, 2021
 *      Author: kjagu
 */

#include "http_server.h"

#define TESTING true
#define MAX_CLIENTS_COUNT 10
#define PING_TIMEOUT_MS 15000

struct async_resp_arg {
    httpd_handle_t hd;	// http server handle
    int fd;				// socket file descriptor
};
struct async_resp_arg ws_clients[MAX_CLIENTS_COUNT];
int ws_clients_count = 0;
bool is_rfid_capture_enabled = 0;

// Tag used for ESP serial console messages
static const char TAG[] = "http_server";

// HTTP server task handle
static httpd_handle_t http_server_handle = NULL;

// HTTP server monitor task handle
static TaskHandle_t task_http_server_monitor = NULL;

// Queue handle used to manipulate the main queue of events
static QueueHandle_t http_server_monitor_queue_handle;

// Embedded files: JQuery, index.html, styles.css, scripts.js and favicon.ico files
//extern const uint8_t jquery_3_3_1_min_js_start[]		asm("_binary_jquery_3_3_1_min_js_start");
//extern const uint8_t jquery_3_3_1_min_js_end[]		asm("_binary_jquery_3_3_1_min_js_end");
extern const uint8_t index_html_start[]				asm("_binary_index_html_start");
extern const uint8_t index_html_end[]				asm("_binary_index_html_end");
extern const uint8_t styles_css_start[]				asm("_binary_styles_css_start");
extern const uint8_t styles_css_end[]				asm("_binary_styles_css_end");
extern const uint8_t scripts_js_start[]				asm("_binary_scripts_js_start");
extern const uint8_t scripts_js_end[]				asm("_binary_scripts_js_end");
extern const uint8_t favicon_ico_start[]			asm("_binary_favicon_ico_start");
extern const uint8_t favicon_ico_end[]				asm("_binary_favicon_ico_end");
extern const uint8_t satoshi_medium_ttf_start[]			asm("_binary_satoshi_medium_ttf_start");
extern const uint8_t satoshi_medium_ttf_end[]				asm("_binary_satoshi_medium_ttf_end");
extern const uint8_t chart_js_start[]						asm("_binary_chart_js_start");
extern const uint8_t chart_js_end[]						asm("_binary_chart_js_end");
extern const uint8_t fns_date_adapter_js_start[]			asm("_binary_fns_date_adapter_js_start");
extern const uint8_t fns_date_adapter_js_end[]			asm("_binary_fns_date_adapter_js_end");

/**
 * HTTP server monitor task used to track events of the HTTP server
 * @param pvParameters parameter which can be passed to the task.
 */
static void http_server_monitor(void *parameter)
{
	http_server_queue_message_t msg;

	for (;;)
	{
		if (xQueueReceive(http_server_monitor_queue_handle, &msg, portMAX_DELAY))
		{
			switch (msg.msgID)
			{
				case HTTP_MSG_WIFI_CONNECT_INIT:
					ESP_LOGI(TAG, "HTTP_MSG_WIFI_CONNECT_INIT");

					break;

				case HTTP_MSG_WIFI_CONNECT_SUCCESS:
					ESP_LOGI(TAG, "HTTP_MSG_WIFI_CONNECT_SUCCESS");

					break;

				case HTTP_MSG_WIFI_CONNECT_FAIL:
					ESP_LOGI(TAG, "HTTP_MSG_WIFI_CONNECT_FAIL");

					break;

				case HTTP_MSG_OTA_UPDATE_SUCCESSFUL:
					ESP_LOGI(TAG, "HTTP_MSG_OTA_UPDATE_SUCCESSFUL");

					break;

				case HTTP_MSG_OTA_UPDATE_FAILED:
					ESP_LOGI(TAG, "HTTP_MSG_OTA_UPDATE_FAILED");

					break;

				case HTTP_MSG_OTA_UPATE_INITIALIZED:
					ESP_LOGI(TAG, "HTTP_MSG_OTA_UPATE_INITIALIZED");

					break;

				default:
					break;
			}
		}
	}
}

static void ws_send_rfid_data(void *arg, esp_event_base_t base, int32_t event_id, void *data) {
	rfid_data_t* rfid_data = malloc(sizeof(rfid_data_t));

	rc522_picc_state_changed_event_t *event = (rc522_picc_state_changed_event_t *)data;
	rc522_picc_t *picc = event->picc;

	if (picc->state != RC522_PICC_STATE_ACTIVE) {
		return;
	}

	rc522_picc_uid_to_str(&picc->uid, rfid_data->uid, sizeof(rfid_data->uid));

	// Comparer l'UID détecté avec l'UID attendu
	if (strcmp(rfid_data->uid, EXPECTED_UID) == 0) {
		rfid_data->is_valid = true;
	} else {
		rfid_data->is_valid = false;
	}
	
	char json_data[128];    
	snprintf(json_data, sizeof(json_data), "{\"data\":{\"uid\":\"%s\", \"is_valid\":\"%s\"}}", rfid_data->uid, rfid_data->is_valid ? "1" : "0");

	// Send data to all clients
    for (int i = 0; i < ws_clients_count; i++) {
        httpd_ws_frame_t ws_pkt;
        memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
        ws_pkt.payload = (uint8_t*)json_data;
        ws_pkt.len = strlen(json_data);
        ws_pkt.type = HTTPD_WS_TYPE_TEXT;

        httpd_ws_send_frame_async(ws_clients[i].hd, ws_clients[i].fd, &ws_pkt);

		//Remove disconnected clients from list
		esp_err_t ret = httpd_ws_send_frame_async(ws_clients[i].hd, ws_clients[i].fd, &ws_pkt);
        
        if (ret != ESP_OK) {
            ESP_LOGW(TAG, "Failed to send WebSocket data to client %d, removing client", i);

			// Remove client and shift remaining clients
			for (int j = i; j < ws_clients_count - 1; j++) {
                ws_clients[j] = ws_clients[j + 1];
            }
            ws_clients_count--;

			//Close connection with client
			if (ws_clients[i].hd != NULL && ws_clients[i].fd >= 0) {
				close(ws_clients[i].fd);
				ESP_LOGI(TAG, "Closed WebSocket connection (fd: %d)", ws_clients[i].fd);
			}
            i--;  // Decrement i because the list size has changed after removal
        }
    }

    free(rfid_data);
}

static void ws_simulate_send_rfid_data(void *pvParameter) {
	rfid_data_t* rfid_data = malloc(sizeof(rfid_data_t));
    char json_data[128];
    while (1) {
        rfid_data = get_random_rfid();
        snprintf(json_data, sizeof(json_data), "{\"data\":{\"uid\":\"%s\", \"is_valid\":\"%s\"}}", rfid_data->uid, rfid_data->is_valid ? "1" : "0");
		ESP_LOGI(TAG, "SENDING %s", json_data);
	// Send data to all clients
    for (int i = 0; i < ws_clients_count; i++) {
        httpd_ws_frame_t ws_pkt;
        memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
        ws_pkt.payload = (uint8_t*)json_data;
        ws_pkt.len = strlen(json_data);
        ws_pkt.type = HTTPD_WS_TYPE_TEXT;

        httpd_ws_send_frame_async(ws_clients[i].hd, ws_clients[i].fd, &ws_pkt);

		//Remove disconnected clients from list
		esp_err_t ret = httpd_ws_send_frame_async(ws_clients[i].hd, ws_clients[i].fd, &ws_pkt);
        
        if (ret != ESP_OK) {
            ESP_LOGW(TAG, "Failed to send WebSocket data to client %d, removing client", i);

			// Remove client and shift remaining clients
			for (int j = i; j < ws_clients_count - 1; j++) {
                ws_clients[j] = ws_clients[j + 1];
            }
            ws_clients_count--;

			//Close connection with client
			if (ws_clients[i].hd != NULL && ws_clients[i].fd >= 0) {
				close(ws_clients[i].fd);
				ESP_LOGI(TAG, "Closed WebSocket connection (fd: %d)", ws_clients[i].fd);
			}
            i--;  // Decrement i because the list size has changed after removal
        }
    }

    free(rfid_data);

        vTaskDelay(pdMS_TO_TICKS(3000));
    }
}


static void ws_async_send(void *arg) {
    static const char * data = "Async data";
    struct async_resp_arg *resp_arg = arg;
    httpd_handle_t hd = resp_arg->hd;
    int fd = resp_arg->fd;
    httpd_ws_frame_t ws_pkt;
    memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
    ws_pkt.payload = (uint8_t*)data;
    ws_pkt.len = strlen(data);
    ws_pkt.type = HTTPD_WS_TYPE_TEXT;

    httpd_ws_send_frame_async(hd, fd, &ws_pkt);
    free(resp_arg);
}

static esp_err_t trigger_async_send(httpd_handle_t handle, httpd_req_t *req)
{
    struct async_resp_arg *resp_arg = malloc(sizeof(struct async_resp_arg));
    if (resp_arg == NULL) {
        return ESP_ERR_NO_MEM;
    }
    resp_arg->hd = req->handle;
    resp_arg->fd = httpd_req_to_sockfd(req);
    esp_err_t ret = httpd_queue_work(handle, ws_async_send, resp_arg);
    if (ret != ESP_OK) {
        free(resp_arg);
    }
    return ret;
}

/*
 * This handler echos back the received ws data
 * and triggers an async send if certain message received
 */
static esp_err_t ws_open_connection(httpd_req_t *req)
{
    if (req->method == HTTP_GET) {
        ESP_LOGI(TAG, "Handshake done, the new connection was opened");

		// Check if the client already exists in the ws_clients array
		for (int i = 0; i < ws_clients_count; i++) {
			if (ws_clients[i].fd == httpd_req_to_sockfd(req)) {
				ESP_LOGI(TAG, "Client already exists");
				return ESP_OK;
			}
		}

		if (ws_clients_count < MAX_CLIENTS_COUNT) {
            ws_clients[ws_clients_count].hd = req->handle;
            ws_clients[ws_clients_count].fd = httpd_req_to_sockfd(req);
            ws_clients_count++;
        } else {
            ESP_LOGW(TAG, "Max clients reached, rejecting new connection");
            return ESP_ERR_NO_MEM;
        }

        return ESP_OK;
    }
    httpd_ws_frame_t ws_pkt;
    uint8_t *buf = NULL;
    memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
    ws_pkt.type = HTTPD_WS_TYPE_TEXT;
    /* Set max_len = 0 to get the frame len */
    esp_err_t ret = httpd_ws_recv_frame(req, &ws_pkt, 0);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "httpd_ws_recv_frame failed to get frame len with %d", ret);
        return ret;
    }
    ESP_LOGI(TAG, "frame len is %d", ws_pkt.len);
    if (ws_pkt.len) {
        /* ws_pkt.len + 1 is for NULL termination as we are expecting a string */
        buf = calloc(1, ws_pkt.len + 1);
        if (buf == NULL) {
            ESP_LOGE(TAG, "Failed to calloc memory for buf");
            return ESP_ERR_NO_MEM;
        }
        ws_pkt.payload = buf;
        /* Set max_len = ws_pkt.len to get the frame payload */
        ret = httpd_ws_recv_frame(req, &ws_pkt, ws_pkt.len);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "httpd_ws_recv_frame failed with %d", ret);
            free(buf);
            return ret;
        }
        ESP_LOGI(TAG, "Got packet with message: %s", ws_pkt.payload);
    }
    ESP_LOGI(TAG, "Packet type: %d", ws_pkt.type);
    if (ws_pkt.type == HTTPD_WS_TYPE_TEXT && strcmp((char*)ws_pkt.payload,"Trigger async") == 0) {
        free(buf);
        return trigger_async_send(req->handle, req);
    }
	else if (ws_pkt.type == HTTPD_WS_TYPE_TEXT && strcmp((char*)ws_pkt.payload, "ping") == 0) {
        ws_pkt.type = HTTPD_WS_TYPE_PONG;
        ret = httpd_ws_send_frame(req, &ws_pkt);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to send WebSocket pong frame");
            return ret;
        }
    }

    ret = httpd_ws_send_frame(req, &ws_pkt);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "httpd_ws_send_frame failed with %d", ret);
    }
    free(buf);
    return ret;
}

/**
 * Jquery get handler is requested when accessing the web page.
 * @param req HTTP request for which the uri needs to be handled.
 * @return ESP_OK
 */
/*
static esp_err_t http_server_jquery_handler(httpd_req_t *req)
{
	ESP_LOGI(TAG, "Jquery requested");

	httpd_resp_set_type(req, "application/javascript");
	httpd_resp_send(req, (const char *)jquery_3_3_1_min_js_start, jquery_3_3_1_min_js_end - jquery_3_3_1_min_js_start);

	return ESP_OK;
}
*/

/**
 * Sends the index.html page.
 * @param req HTTP request for which the uri needs to be handled.
 * @return ESP_OK
 */
static esp_err_t http_server_index_html_handler(httpd_req_t *req) {
	ESP_LOGI(TAG, "index.html requested");

	httpd_resp_set_type(req, "text/html");
	httpd_resp_send(req, (const char *)index_html_start, index_html_end - index_html_start);

	return ESP_OK;
}

/**
 * Sends JSON data.
 * @param req HTTP request for which the uri needs to be handled.
 * @return ESP_OK
 */
static esp_err_t http_server_get_temp_handler(httpd_req_t *req)
{
	ESP_LOGI(TAG, "Temperature data requested");

    char sensor_data[16];
    char json_data[128];
    snprintf(sensor_data, sizeof(sensor_data), "%.2f", TESTING ? simulate_temp() : get_temp());
    snprintf(json_data, sizeof(json_data), "{\"data\":\"%s\"}", sensor_data);

	httpd_resp_set_type(req, "application/json");
	httpd_resp_send(req, json_data, strlen(json_data));

	return ESP_OK;
}

/**
 * Sends JSON data.
 * @param req HTTP request for which the uri needs to be handled.
 * @return ESP_OK
 */
static esp_err_t http_server_get_gas_handler(httpd_req_t *req)
{
	ESP_LOGI(TAG, "Gas data requested");

    char sensor_data[16];
    char json_data[128];
    snprintf(sensor_data, sizeof(sensor_data), "%.2f", TESTING ? simulate_gaz() : get_gas());
    snprintf(json_data, sizeof(json_data), "{\"data\":\"%s\"}", sensor_data);

	httpd_resp_set_type(req, "application/json");
	httpd_resp_send(req, json_data, strlen(json_data));

	return ESP_OK;
}

/**
 * Sends JSON data.
 * @param req HTTP request for which the uri needs to be handled.
 * @return ESP_OK
 */
static esp_err_t http_server_get_movement_handler(httpd_req_t *req)
{
	ESP_LOGI(TAG, "Movement data requested");

    char sensor_data[16];
    char json_data[128];
    snprintf(sensor_data, sizeof(sensor_data), "%.2f", (float)(TESTING ? simulate_movement() : get_movement()));
    snprintf(json_data, sizeof(json_data), "{\"data\":\"%s\"}", sensor_data);

	httpd_resp_set_type(req, "application/json");
	httpd_resp_send(req, json_data, strlen(json_data));

	return ESP_OK;
}

/**
 * Sends JSON data.
 * @param req HTTP request for which the uri needs to be handled.
 * @return ESP_OK
 */
static esp_err_t http_server_get_rfid_handler(httpd_req_t *req)
{
	//TODO make rfid send data using webSockets
	ESP_LOGI(TAG, "RFID data requested");
	
	if (is_rfid_capture_enabled) {
    	if (TESTING) {
			xTaskCreate(ws_simulate_send_rfid_data, "ws_simulate_send_rfid_data", 4096, NULL, 1, NULL);
		} else {
			start_rfid();
			capture_rfid(ws_send_rfid_data);
		}
		is_rfid_capture_enabled = 1;
	}
    ws_open_connection(req);

	return ESP_OK;
}

/**
 * styles.css get handler is requested when accessing the web page.
 * @param req HTTP request for which the uri needs to be handled.
 * @return ESP_OK
 */
static esp_err_t http_server_styles_css_handler(httpd_req_t *req)
{
	ESP_LOGI(TAG, "styles.css requested");

	httpd_resp_set_type(req, "text/css");
	httpd_resp_send(req, (const char *)styles_css_start, styles_css_end - styles_css_start);

	return ESP_OK;
}

/**
 * scripts.js get handler is requested when accessing the web page.
 * @param req HTTP request for which the uri needs to be handled.
 * @return ESP_OK
 */
static esp_err_t http_server_scripts_js_handler(httpd_req_t *req)
{
	ESP_LOGI(TAG, "scripts.js requested");

	httpd_resp_set_type(req, "application/javascript");
	httpd_resp_send(req, (const char *)scripts_js_start, scripts_js_end - scripts_js_start);

	return ESP_OK;
}

/**
 * satoshi-medium.ttf get handler is requested when accessing the web page.
 * @param req HTTP request for which the uri needs to be handled.
 * @return ESP_OK
 */
static esp_err_t http_server_satoshi_medium_ttf_handler(httpd_req_t *req)
{
	ESP_LOGI(TAG, "satoshi-medium.ttf requested");

	httpd_resp_set_type(req, "application/javascript");
	httpd_resp_send(req, (const char *)satoshi_medium_ttf_start, satoshi_medium_ttf_end - satoshi_medium_ttf_start);

	return ESP_OK;
}

/**
 * chart.js get handler is requested when accessing the web page.
 * @param req HTTP request for which the uri needs to be handled.
 * @return ESP_OK
 */
static esp_err_t http_server_chart_js_handler(httpd_req_t *req)
{
	ESP_LOGI(TAG, "chart.js requested");

	httpd_resp_set_type(req, "application/javascript");
	httpd_resp_send(req, (const char *)chart_js_start, chart_js_end - chart_js_start);

	return ESP_OK;
}

/**
 * fns-date-adapter.js get handler is requested when accessing the web page.
 * @param req HTTP request for which the uri needs to be handled.
 * @return ESP_OK
 */
static esp_err_t http_server_fns_date_adapter_js_handler(httpd_req_t *req)
{
	ESP_LOGI(TAG, "fns-date-adapter.js requested");

	httpd_resp_set_type(req, "application/javascript");
	httpd_resp_send(req, (const char *)fns_date_adapter_js_start, fns_date_adapter_js_end - fns_date_adapter_js_start);

	return ESP_OK;
}

/**
 * Sends the .ico (icon) file when accessing the web page.
 * @param req HTTP request for which the uri needs to be handled.
 * @return ESP_OK
 */
static esp_err_t http_server_favicon_ico_handler(httpd_req_t *req)
{
	ESP_LOGI(TAG, "favicon.ico requested");

	httpd_resp_set_type(req, "image/x-icon");
	httpd_resp_send(req, (const char *)favicon_ico_start, favicon_ico_end - favicon_ico_start);

	return ESP_OK;
}

/**
 * Sets up the default httpd server configuration.
 * @return http server instance handle if successful, NULL otherwise.
 */
static httpd_handle_t http_server_configure(void)
{
	// Generate the default configuration
	httpd_config_t config = HTTPD_DEFAULT_CONFIG();

	// Create HTTP server monitor task
	xTaskCreatePinnedToCore(&http_server_monitor, "http_server_monitor", HTTP_SERVER_MONITOR_STACK_SIZE, NULL, HTTP_SERVER_MONITOR_PRIORITY, &task_http_server_monitor, HTTP_SERVER_MONITOR_CORE_ID);

	// Create the message queue
	http_server_monitor_queue_handle = xQueueCreate(3, sizeof(http_server_queue_message_t));

	// The core that the HTTP server will run on
	config.core_id = HTTP_SERVER_TASK_CORE_ID;

	// Adjust the default priority to 1 less than the wifi application task
	config.task_priority = HTTP_SERVER_TASK_PRIORITY;

	// Bump up the stack size (default is 4096)
	config.stack_size = HTTP_SERVER_TASK_STACK_SIZE;

	// Increase uri handlers
	config.max_uri_handlers = 20;

	// Increase the timeout limits
	config.recv_wait_timeout = 10;
	config.send_wait_timeout = 10;

	ESP_LOGI(TAG,
			"http_server_configure: Starting server on port: '%d' with task priority: '%d'",
			config.server_port,
			config.task_priority);

	// Start the httpd server
	if (httpd_start(&http_server_handle, &config) == ESP_OK)
	{
		ESP_LOGI(TAG, "http_server_configure: Registering URI handlers");
/*
		// register query handler
		httpd_uri_t jquery_js = {
				.uri = "/jquery-3.3.1.min.js",
				.method = HTTP_GET,
				.handler = http_server_jquery_handler,
				.user_ctx = NULL
		};
		httpd_register_uri_handler(http_server_handle, &jquery_js);
*/

		httpd_uri_t index_html = {
				.uri = "/",
				.method = HTTP_GET,
				.handler = http_server_index_html_handler,
				.user_ctx = NULL
		};
		httpd_register_uri_handler(http_server_handle, &index_html);

		httpd_uri_t get_temp = {
				.uri = "/api/get/temperature",
				.method = HTTP_GET,
				.handler = http_server_get_temp_handler,
				.user_ctx = NULL
		};
		httpd_register_uri_handler(http_server_handle, &get_temp);

		httpd_uri_t get_gas = {
				.uri = "/api/get/gas",
				.method = HTTP_GET,
				.handler = http_server_get_gas_handler,
				.user_ctx = NULL
		};
		httpd_register_uri_handler(http_server_handle, &get_gas);

		httpd_uri_t get_movement = {
				.uri = "/api/get/movement",
				.method = HTTP_GET,
				.handler = http_server_get_movement_handler,
				.user_ctx = NULL
		};
		httpd_register_uri_handler(http_server_handle, &get_movement);

		httpd_uri_t get_rfid = {
				.uri = "/api/ws/rfid",
				.method = HTTP_GET,
				.handler = http_server_get_rfid_handler,
				.user_ctx = NULL,
				.is_websocket = true, 
		};
		httpd_register_uri_handler(http_server_handle, &get_rfid);

		httpd_uri_t styles_css = {
				.uri = "/styles.css",
				.method = HTTP_GET,
				.handler = http_server_styles_css_handler,
				.user_ctx = NULL
		};
		httpd_register_uri_handler(http_server_handle, &styles_css);

		httpd_uri_t scripts_js = {
				.uri = "/scripts.js",
				.method = HTTP_GET,
				.handler = http_server_scripts_js_handler,
				.user_ctx = NULL
		};
		httpd_register_uri_handler(http_server_handle, &scripts_js);

		httpd_uri_t satoshi_medium_ttf = {
				.uri = "/fonts/satoshi-medium.ttf",
				.method = HTTP_GET,
				.handler = http_server_satoshi_medium_ttf_handler,
				.user_ctx = NULL
		};
		httpd_register_uri_handler(http_server_handle, &satoshi_medium_ttf);

		httpd_uri_t chart_js = {
				.uri = "/libs/chart.js",
				.method = HTTP_GET,
				.handler = http_server_chart_js_handler,
				.user_ctx = NULL
		};
		httpd_register_uri_handler(http_server_handle, &chart_js);

		httpd_uri_t fns_date_adapter_js = {
				.uri = "/libs/fns-date-adapter.js",
				.method = HTTP_GET,
				.handler = http_server_fns_date_adapter_js_handler,
				.user_ctx = NULL
		};
		httpd_register_uri_handler(http_server_handle, &fns_date_adapter_js);

		httpd_uri_t favicon_ico = {
				.uri = "/favicon.ico",
				.method = HTTP_GET,
				.handler = http_server_favicon_ico_handler,
				.user_ctx = NULL
		};
		httpd_register_uri_handler(http_server_handle, &favicon_ico);

		return http_server_handle;
	}

	return NULL;
}

void http_server_start(void)
{
	if (http_server_handle == NULL)
	{
		http_server_handle = http_server_configure();
	}
}

void http_server_stop(void)
{
	if (http_server_handle)
	{
		httpd_stop(http_server_handle);
		ESP_LOGI(TAG, "http_server_stop: stopping HTTP server");
		http_server_handle = NULL;
	}
	if (task_http_server_monitor)
	{
		vTaskDelete(task_http_server_monitor);
		ESP_LOGI(TAG, "http_server_stop: stopping HTTP server monitor");
		task_http_server_monitor = NULL;
	}
}

BaseType_t http_server_monitor_send_message(http_server_message_e msgID)
{
	http_server_queue_message_t msg;
	msg.msgID = msgID;
	return xQueueSend(http_server_monitor_queue_handle, &msg, portMAX_DELAY);
}



















