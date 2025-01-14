#include "idir_wifi.h"
#define MAX_APs 10//si en a une zonne plusieurs d'accé en vas limiter  10 point d'ccé a scaner 
static int32_t connected=0;//suivre etat de connection
esp_event_handler_instance_t interface_ip;
esp_event_handler_instance_t interface_wifi;

esp_netif_t* netif_sta=NULL;//nom de l'interface réseau en vas configurer par la suit
esp_netif_t* netif_ap=NULL;//interface AP
//fonction de réponce 
void tache_scan(void*par){
  wifi_scan_config_t wifiscanconfig={//structure de configuation pour le scan
        .ssid=NULL,
        .bssid=NULL,//MAC
        .channel=0,
        .show_hidden=true//afficher les AP caché
    };
     while(true){
           if(connected==0){
            printf("début de scan \n");
            esp_wifi_scan_start(&wifiscanconfig,true);
            printf("scan done \n");
           uint16_t ap_count=0;//récupérer nombre de point d'accé diponible
           ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&ap_count));//retrourne nomnbre de point d'accé trouvé 
            if(ap_count==0){
              printf("aucun point d'accé trouvé \n");
            }else{
              wifi_ap_record_t ap_records[MAX_APs];
              printf("nbr de point d'accé est : %d \n",ap_count);
              esp_wifi_scan_get_ap_records(&ap_count,ap_records);
              for(int i=0;i<ap_count;i++){
                printf("ssid %s\n  | rssi %d mdb\n | channel %d\n | authomode  %d \n\n\n",
                              ap_records[i].ssid,
                              ap_records[i].rssi,
                              ap_records[i].primary,
                              ap_records[i].authmode
                );
                printf("**********************************\n");
                if(strcmp((char*)ap_records[i].ssid,"Idir_hack")==0){//si notre point d'accé et reconue se connecter
                  esp_wifi_connect();
                }
              }
            }
           }
      vTaskDelay(pdMS_TO_TICKS(1000));
       }
}
void handler(void* hand, esp_event_base_t event, int32_t id_event, void* event_data) {
    if (event == WIFI_EVENT) {
        switch (id_event) {
            case WIFI_EVENT_AP_START:
                printf("Access point started\n");
                break;
            case WIFI_EVENT_AP_STACONNECTED:
                printf("A device is connected\n");
                break;
            case WIFI_EVENT_STA_START:
                  printf("la station a démaré \n");
                  printf("rechercher le point d'accé \n");
                  xTaskCreate(tache_scan,"scan tache",4096,NULL,5,NULL);//creer la tache de scan
                  break;
            case WIFI_EVENT_STA_DISCONNECTED:
                  printf("connexion perdue n \n");
                  printf("reconnextion \n");
                  connected=0;
                  break;
                break;
        }
    } else if (event == IP_EVENT) {
        switch (id_event) {
            case IP_EVENT_STA_GOT_IP:
                 printf("station est connecté.. \n");
               //afficher la confuguration IP
                ip_event_got_ip_t* ip_info2=(ip_event_got_ip_t*)event_data;//caster l'address IP
                printf("Adresse IP obtenue : " IPSTR "\n", IP2STR(&ip_info2->ip_info.ip));
                printf("Masque de sous-réseau : " IPSTR "\n", IP2STR(&ip_info2->ip_info.netmask));
                printf("Passerelle : " IPSTR "\n", IP2STR(&ip_info2->ip_info.gw));
             connected=1;//mettre la valeur de connected a 1
                break;
            case IP_EVENT_AP_STAIPASSIGNED:
                printf("A client got an IP address\n");
                break;
        }
    }
}
void init_wifi(void){
    esp_netif_init();
    nvs_flash_init();
    esp_event_loop_create_default();
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &handler, NULL, &interface_wifi));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,ESP_EVENT_ANY_ID, &handler, NULL, &interface_ip));
   // ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    netif_ap = esp_netif_create_default_wifi_ap();
    netif_sta=esp_netif_create_default_wifi_sta();
    wifi_init_config_t config1 = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&config1));
      wifi_config_t config_sta={
      .sta={
        .ssid="Idir_hack",
        .password="1234idir"
      },
  };
    wifi_config_t config6 = {
        .ap = {
            .channel = 0,
            .ssid = "esp_idir",
            .max_connection = 4,
            .password = "123456789",
            .authmode = WIFI_AUTH_WPA2_WPA3_PSK,
            .ssid_hidden = false,
        }
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &config6));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &config_sta));

    // DHCP server configuration
    esp_netif_ip_info_t ip_info;
    IP4_ADDR(&ip_info.ip, 192, 168, 4, 1);
    IP4_ADDR(&ip_info.gw, 192, 168, 4, 1);
    IP4_ADDR(&ip_info.netmask, 255, 255, 255, 0);
    ESP_ERROR_CHECK( esp_netif_dhcps_stop(netif_ap)); // Stop to configure
    esp_netif_set_ip_info(netif_ap, &ip_info); // Set the IP range
    esp_netif_dhcps_start(netif_ap); // Start the DHCP server after configuration
    esp_wifi_start();
}





