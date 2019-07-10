#include "mgos.h"
#include "mgos_mqtt.h"
#include "mgos_wifi.h"

#include "Adafruit_BME280.h"

#define SENSOR_ADDR 0x76

static void net_cb(int ev, void *evd, void *arg) {
  switch (ev) {
    case MGOS_NET_EV_DISCONNECTED:
      LOG(LL_INFO, ("%s", "Net disconnected"));
      break;
    case MGOS_NET_EV_CONNECTING:
      LOG(LL_INFO, ("%s", "Net connecting..."));
      break;
    case MGOS_NET_EV_CONNECTED:
      LOG(LL_INFO, ("%s", "Net connected"));
      break;
    case MGOS_NET_EV_IP_ACQUIRED:
      LOG(LL_INFO, ("%s", "Net got IP address"));
      break;
  }

  (void) evd;
  (void) arg;
}

static void wifi_cb(int ev, void *evd, void *arg) {
  switch (ev) {
    case MGOS_WIFI_EV_STA_DISCONNECTED: {
      struct mgos_wifi_sta_disconnected_arg *da =
          (struct mgos_wifi_sta_disconnected_arg *) evd;
      LOG(LL_INFO, ("WiFi STA disconnected, reason %d", da->reason));
      break;
    }
    case MGOS_WIFI_EV_STA_CONNECTING:
      LOG(LL_INFO, ("WiFi STA connecting %p", arg));
      break;
    case MGOS_WIFI_EV_STA_CONNECTED:
      LOG(LL_INFO, ("WiFi STA connected %p", arg));
      break;
    case MGOS_WIFI_EV_STA_IP_ACQUIRED:
      LOG(LL_INFO, ("WiFi STA IP acquired %p", arg));
      break;
    case MGOS_WIFI_EV_AP_STA_CONNECTED: {
      struct mgos_wifi_ap_sta_connected_arg *aa =
          (struct mgos_wifi_ap_sta_connected_arg *) evd;
      LOG(LL_INFO, ("WiFi AP STA connected MAC %02x:%02x:%02x:%02x:%02x:%02x",
                    aa->mac[0], aa->mac[1], aa->mac[2], aa->mac[3], aa->mac[4],
                    aa->mac[5]));
      break;
    }
    case MGOS_WIFI_EV_AP_STA_DISCONNECTED: {
      struct mgos_wifi_ap_sta_disconnected_arg *aa =
          (struct mgos_wifi_ap_sta_disconnected_arg *) evd;
      LOG(LL_INFO,
          ("WiFi AP STA disconnected MAC %02x:%02x:%02x:%02x:%02x:%02x",
           aa->mac[0], aa->mac[1], aa->mac[2], aa->mac[3], aa->mac[4],
           aa->mac[5]));
      break;
    }
  }
  (void) arg;
}

static const char* get_device_id() {
  return mgos_sys_config_get_device_id();
}

static void log_device_stats(const char* device_id) {
  LOG(LL_INFO,
      ("%s - uptime: %.2lf, RAM: %lu, %lu free", device_id,
       mgos_uptime(), (unsigned long) mgos_get_heap_size(),
       (unsigned long) mgos_get_free_heap_size()));
}

static void timer_cb(void *arg) {
  const char* device_id = get_device_id();
  log_device_stats(device_id);

  Adafruit_BME280 *bme = new Adafruit_BME280();

  if (!bme->begin(SENSOR_ADDR)) {
    LOG(LL_ERROR, ("Can't find sensor, skipping iteration"));
    return;
  }

  LOG(LL_INFO, ("Found sensor. Waiting 2 seconds before taking readings..."));
  delay(2000);

  float temp = bme->readTemperature();
  float humidity = bme->readHumidity();
  float pressure = bme->readPressure() / 1000.0;

  delete bme;

  LOG(LL_INFO, ("Readings - temp %.2f *C, humidity %.2f %%RH, pressure %.2f kPa",
                temp, humidity, pressure));

  (void) arg;
}

enum mgos_app_init_result mgos_app_init(void) {

  mgos_event_add_group_handler(MGOS_EVENT_GRP_NET, net_cb, NULL);
  mgos_event_add_group_handler(MGOS_WIFI_EV_BASE, wifi_cb, NULL);

  mgos_set_timer(20000, MGOS_TIMER_REPEAT, timer_cb, NULL);

  return MGOS_APP_INIT_SUCCESS;
}

