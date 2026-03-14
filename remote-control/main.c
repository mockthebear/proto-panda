#include "nrf_drv_clock.h"

#include "settings.h"

#include <stdint.h>

#include <string.h>

#include "ble_nus.h"

#include "LIS3DHTR.h"

#define PIN_LED_1 2
#define PIN_LED_2 3
#define PIN_POWER_ACC 4

const uint8_t button_pins[] = {
  30,
  31,
  7,
  6,
  5,
  29,
};

static int gmode = 0;

static int adv_led = 0;
static bool use_led = true;
static bool loud_led = true;
static bool hasLis3 = false;
static bool started_everything = false;
static bool received_config_integer = false;
static uint16_t config_integer = 0xffff;
static ble_os_t m_our_service;
static int rebeginCycles;


NRF_BLE_GATT_DEF(m_gatt); /**< GATT module instance. */
NRF_BLE_QWR_DEF(m_qwr); /**< Context for the Queued Write module.*/
APP_TIMER_DEF(m_app_timer_id); /**< Handle of the current connection. */

static uint8_t m_adv_handle = BLE_GAP_ADV_SET_HANDLE_NOT_SET; /**< Advertising handle used to identify an advertising set. */
static uint8_t m_enc_advdata[BLE_GAP_ADV_SET_DATA_SIZE_MAX]; /**< Buffer for storing an encoded advertising set. */
static uint8_t m_enc_scan_response_data[BLE_GAP_ADV_SET_DATA_SIZE_MAX]; /**< Buffer for storing an encoded scan data. */
static int32_t advCountToSleep = 0;
static int32_t started_timer = 0;
static int16_t AccValue[STREAM_SIZE];
static uint8_t button_states[10];
static int blinkTimer = 0;
static int cycleTimer = PIN_LED_1;
static bool canCycle = false;

/**@brief Struct that contains pointers to the encoded advertising data. */
static ble_gap_adv_data_t m_adv_data = {
  .adv_data = {
    .p_data = m_enc_advdata,
    .len = BLE_GAP_ADV_SET_DATA_SIZE_MAX
  },
  .scan_rsp_data = {
    .p_data = m_enc_scan_response_data,
    .len = BLE_GAP_ADV_SET_DATA_SIZE_MAX

  }
};

extern void twi_master_init(void);

static void log_init() {
  ret_code_t err_code = NRF_LOG_INIT(NULL);
  APP_ERROR_CHECK(err_code);

  NRF_LOG_DEFAULT_BACKENDS_INIT();
}

static void leds_init(void) {
  bsp_board_init(BSP_INIT_LEDS);
}

static void on_conn_params_evt(ble_conn_params_evt_t * p_evt) {
  ret_code_t err_code;

  if (p_evt -> evt_type == BLE_CONN_PARAMS_EVT_FAILED) {
    err_code = sd_ble_gap_disconnect(m_our_service.conn_handle, BLE_HCI_CONN_INTERVAL_UNACCEPTABLE);
    APP_ERROR_CHECK(err_code);
  }
}

static void conn_params_error_handler(uint32_t nrf_error) {
  APP_ERROR_HANDLER(nrf_error);
}

/**@brief Function for initializing the Connection Parameters module.
 */
static void conn_params_init(void) {
  ret_code_t err_code;
  ble_conn_params_init_t cp_init;

  memset( & cp_init, 0, sizeof(cp_init));

  cp_init.p_conn_params = NULL;
  cp_init.first_conn_params_update_delay = FIRST_CONN_PARAMS_UPDATE_DELAY;
  cp_init.next_conn_params_update_delay = NEXT_CONN_PARAMS_UPDATE_DELAY;
  cp_init.max_conn_params_update_count = MAX_CONN_PARAMS_UPDATE_COUNT;
  cp_init.start_on_notify_cccd_handle = BLE_GATT_HANDLE_INVALID;
  cp_init.disconnect_on_fail = false;
  cp_init.evt_handler = on_conn_params_evt;
  cp_init.error_handler = conn_params_error_handler;

  err_code = ble_conn_params_init( & cp_init);
  APP_ERROR_CHECK(err_code);
}

void
assert_nrf_callback(uint16_t line_num,
  const uint8_t * p_file_name) {
  app_error_handler(DEAD_BEEF, line_num, p_file_name);
}

static void gap_params_init(void) {
  ret_code_t err_code;
  ble_gap_conn_params_t gap_conn_params;
  ble_gap_conn_sec_mode_t sec_mode;

  BLE_GAP_CONN_SEC_MODE_SET_OPEN( & sec_mode);

  err_code = sd_ble_gap_device_name_set( & sec_mode, (const uint8_t * ) DEVICE_NAME, strlen(DEVICE_NAME));

  APP_ERROR_CHECK(err_code);

  memset( & gap_conn_params, 0, sizeof(gap_conn_params));

  gap_conn_params.min_conn_interval = MIN_CONN_INTERVAL;
  gap_conn_params.max_conn_interval = MAX_CONN_INTERVAL;
  gap_conn_params.slave_latency = SLAVE_LATENCY;
  gap_conn_params.conn_sup_timeout = CONN_SUP_TIMEOUT;

  err_code = sd_ble_gap_ppcp_set( & gap_conn_params);
  APP_ERROR_CHECK(err_code);
}

/**@brief Function for initializing the Advertising functionality.
 *
 * @details Encodes the required advertising data and passes it to the stack.
 *          Also builds a structure to be passed to the stack when starting advertising.
 */
static void advertising_init(void) {
  ret_code_t err_code;
  ble_advdata_t advdata;
  ble_advdata_t srdata;

  ble_uuid_t adv_uuids[] = {
    {
      MOC_UUID_SERVICE,
      m_our_service.uuid_type
    }
  };

  // Build and set advertising data.
  memset( & advdata, 0, sizeof(advdata));

  advdata.name_type = BLE_ADVDATA_FULL_NAME;
  advdata.include_appearance = true;
  advdata.flags = BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE;

  memset( & srdata, 0, sizeof(srdata));
  srdata.uuids_complete.uuid_cnt = sizeof(adv_uuids) / sizeof(adv_uuids[0]);
  srdata.uuids_complete.p_uuids = adv_uuids;

  err_code = ble_advdata_encode( & advdata, m_adv_data.adv_data.p_data, & m_adv_data.adv_data.len);
  APP_ERROR_CHECK(err_code);

  err_code = ble_advdata_encode( & srdata, m_adv_data.scan_rsp_data.p_data, & m_adv_data.scan_rsp_data.len);
  APP_ERROR_CHECK(err_code);

  ble_gap_adv_params_t adv_params;

  // Set advertising parameters.
  memset( & adv_params, 0, sizeof(adv_params));

  adv_params.primary_phy = BLE_GAP_PHY_1MBPS;
  adv_params.duration = APP_ADV_DURATION;
  adv_params.properties.type = BLE_GAP_ADV_TYPE_CONNECTABLE_SCANNABLE_UNDIRECTED;
  adv_params.p_peer_addr = NULL;
  adv_params.filter_policy = BLE_GAP_ADV_FP_ANY;
  adv_params.interval = APP_ADV_INTERVAL;

  err_code = sd_ble_gap_adv_set_configure( & m_adv_handle, & m_adv_data, & adv_params);
  APP_ERROR_CHECK(err_code);
}

/**@brief Function for handling Queued Write Module errors.
 *
 * @details A pointer to this function will be passed to each service which may need to inform the
 *          application about an error.
 *
 * @param[in]   nrf_error   Error code containing information about what went wrong.
 */
static void nrf_qwr_error_handler(uint32_t nrf_error) {
  APP_ERROR_HANDLER(nrf_error);
}

static void sleep_mode_enter(void) {

  
  nrf_gpio_pin_set(PIN_LED_2);
  nrf_gpio_pin_clear(PIN_LED_1);

  uint32_t err_code;
  for (int i = 0; i < 6; i++) {
    nrf_gpio_cfg_input(button_pins[i], NRF_GPIO_PIN_PULLUP);
    nrf_gpio_cfg_sense_input(button_pins[i], NRF_GPIO_PIN_PULLUP, NRF_GPIO_PIN_SENSE_LOW);
  }
  if (hasLis3) {

    LSM6_set_power_mode(false);
  }
  // NRF_LOG_PROCESS();
  
  nrf_delay_ms(2000);


  nrf_gpio_pin_clear(PIN_LED_2);
  nrf_gpio_pin_clear(PIN_LED_2);
  sd_power_system_off();
}

static void services_init(void) {
  ret_code_t err_code;
  nrf_ble_qwr_init_t qwr_init = {
    0
  };

  qwr_init.error_handler = nrf_qwr_error_handler;

  err_code = nrf_ble_qwr_init( & m_qwr, & qwr_init);
  APP_ERROR_CHECK(err_code);

  ble_uuid_t ble_uuid;

  ble_uuid128_t base_uuida = {
    MOC_UUID_BASE
  };

  err_code = sd_ble_uuid_vs_add( & base_uuida, & m_our_service.uuid_type);

  APP_ERROR_CHECK(err_code);

  ble_uuid.type = m_our_service.uuid_type;

  ble_uuid.uuid = MOC_UUID_SERVICE;

  err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY, & ble_uuid, & m_our_service.service_handle);

  APP_ERROR_CHECK(err_code);

  ble_add_char_params_t add_char_params;

  memset( & add_char_params, 0, sizeof(add_char_params));
  add_char_params.uuid = 0xfafb;
  add_char_params.uuid_type = m_our_service.uuid_type;
  add_char_params.init_len = sizeof(int32_t);
  add_char_params.max_len = sizeof(int32_t);
  add_char_params.char_props.write = 1;
  add_char_params.char_props.read = 1;
  add_char_params.write_access = SEC_OPEN;
  add_char_params.read_access = SEC_OPEN;

  err_code = characteristic_add(m_our_service.service_handle, &
    add_char_params, &
    m_our_service.config_handle);
  APP_ERROR_CHECK(err_code);

  memset( & add_char_params, 0, sizeof(add_char_params));
  add_char_params.uuid = 0xfafaf;
  add_char_params.uuid_type = m_our_service.uuid_type;
  add_char_params.init_len = sizeof(int16_t) * STREAM_SIZE;
  add_char_params.max_len = sizeof(int16_t) * STREAM_SIZE;
  add_char_params.char_props.read = 1;
  add_char_params.char_props.notify = 1;

  add_char_params.read_access = SEC_OPEN;
  add_char_params.cccd_write_access = SEC_OPEN;

  err_code = characteristic_add(m_our_service.service_handle, & add_char_params, & m_our_service.mems_handle);

  APP_ERROR_CHECK(err_code);
}

/**@brief Function for starting advertising.
 */
static void advertising_start(void) {
  received_config_integer = false;
  ret_code_t err_code;
  err_code = sd_ble_gap_adv_start(m_adv_handle, APP_BLE_CONN_CFG_TAG);
  APP_ERROR_CHECK(err_code);
  nrf_gpio_pin_clear(PIN_LED_1);
  nrf_gpio_pin_set(PIN_LED_1);
}

/**@brief Function for handling BLE events.
 *
 * @param[in]   p_ble_evt   Bluetooth stack event.
 * @param[in]   p_context   Unused.
 */
static void ble_evt_handler(ble_evt_t
  const * p_ble_evt, void * p_context) {
  ret_code_t err_code;

  switch (p_ble_evt -> header.evt_id) {
  case BLE_GAP_EVT_CONNECTED:
    NRF_LOG_INFO("Connected");
    advCountToSleep = 0;
    nrf_gpio_pin_clear(adv_led);
    m_our_service.conn_handle = p_ble_evt -> evt.gap_evt.conn_handle;
    err_code = nrf_ble_qwr_conn_handle_assign( & m_qwr, m_our_service.conn_handle);
    APP_ERROR_CHECK(err_code);
    break;

  case BLE_GAP_EVT_DISCONNECTED:
    NRF_LOG_INFO("Disconnected");
    advCountToSleep = 0;
    rebeginCycles = 50;
    m_our_service.conn_handle = BLE_CONN_HANDLE_INVALID;
    advertising_start();
    break;

  case BLE_GAP_EVT_SEC_PARAMS_REQUEST:
    err_code = sd_ble_gap_sec_params_reply(m_our_service.conn_handle, BLE_GAP_SEC_STATUS_PAIRING_NOT_SUPP, NULL, NULL);
    APP_ERROR_CHECK(err_code);
    break;

  case BLE_GAP_EVT_CONN_PARAM_UPDATE: {
        err_code = sd_ble_gap_conn_param_update(p_ble_evt -> evt.gatts_evt.conn_handle, NULL);
        APP_ERROR_CHECK(err_code);
    }
    break;
  case BLE_GAP_EVT_DATA_LENGTH_UPDATE_REQUEST:
    NRF_LOG_DEBUG("Data length update request received");
    // Accept the requested data length parameters
    ble_gap_data_length_params_t dl_params;
    
    // You can either accept the peer's requested parameters or propose your own
    err_code = sd_ble_gap_data_length_update(p_ble_evt->evt.gap_evt.conn_handle, NULL, NULL);
    // Using NULL, NULL accepts the peer's requested parameters
    
    if (err_code != NRF_SUCCESS) {
        NRF_LOG_ERROR("Failed to update data length: %d", err_code);
    }
    break;
  case BLE_GAP_EVT_ADV_REPORT:

    NRF_LOG_DEBUG("Advertising report received from:");

    break;
  case BLE_GAP_EVT_PHY_UPDATE_REQUEST: {
        NRF_LOG_DEBUG("PHY update request.");
        ble_gap_phys_t
        const phys = {
          .rx_phys = BLE_GAP_PHY_AUTO,
          .tx_phys = BLE_GAP_PHY_AUTO,
        };
        err_code = sd_ble_gap_phy_update(p_ble_evt -> evt.gap_evt.conn_handle, & phys);
        APP_ERROR_CHECK(err_code);
      }
      break;
  case BLE_GATTS_EVT_WRITE: {
        ble_gatts_evt_write_t
        const * p_evt_write = & p_ble_evt -> evt.gatts_evt.params.write;

        // Check if this is a write to our config characteristic
        if (p_evt_write -> handle == m_our_service.config_handle.value_handle) {
          if (p_evt_write -> len == sizeof(int32_t)) {
            // Store the received integer in the first element of AccValue
            int32_t received_value;
            memcpy( & received_value, p_evt_write -> data, sizeof(int32_t));
            config_integer = (int16_t) received_value; // Store in first element
            received_config_integer = true;
            NRF_LOG_INFO("Received config integer: %d", received_value);
          }
        }
      }
    break;
  case BLE_GATTS_EVT_SYS_ATTR_MISSING:
    err_code = sd_ble_gatts_sys_attr_set(m_our_service.conn_handle, NULL, 0, 0);
    APP_ERROR_CHECK(err_code);
    break;
  case BLE_GATTC_EVT_EXCHANGE_MTU_RSP:
  {
      NRF_LOG_INFO("GATT Client MTU exchange response received");
    
      // Get the negotiated MTU from the event
      ble_gattc_evt_exchange_mtu_rsp_t *p_mtu_rsp = 
          &p_ble_evt->evt.gattc_evt.params.exchange_mtu_rsp;
    
      // The negotiated MTU is available in the response
      // The SoftDevice automatically sets ATT_MTU to the minimum of:
      // - Client RX MTU value you requested, and
      // - Server RX MTU value from this response [citation:1][citation:3]
      uint16_t negotiated_mtu = p_mtu_rsp->server_rx_mtu;
    
      NRF_LOG_INFO("Negotiated MTU: %d", negotiated_mtu);
    
      // You can store this value if needed for your application
      // current_mtu = negotiated_mtu;
    
      // The ATT MTU is now updated - you can proceed with larger data transfers
      break;
  }
  case BLE_GATTC_EVT_TIMEOUT:
    NRF_LOG_DEBUG("GATT Client Timeout.");
    err_code = sd_ble_gap_disconnect(p_ble_evt -> evt.gattc_evt.conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
    APP_ERROR_CHECK(err_code);
    break;

  case BLE_GATTS_EVT_TIMEOUT:
    // Disconnect on GATT Server timeout event.
    NRF_LOG_DEBUG("GATT Server Timeout.");
    err_code = sd_ble_gap_disconnect(p_ble_evt -> evt.gatts_evt.conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
    APP_ERROR_CHECK(err_code);
    break;
  case BLE_GATTS_EVT_EXCHANGE_MTU_REQUEST:
    NRF_LOG_DEBUG("MTU exchange request received");
    err_code = sd_ble_gatts_exchange_mtu_reply(p_ble_evt->evt.gatts_evt.conn_handle, NRF_SDH_BLE_GATT_MAX_MTU_SIZE);
    APP_ERROR_CHECK(err_code);
    NRF_LOG_INFO("Responded with MTU: %d", NRF_SDH_BLE_GATT_MAX_MTU_SIZE);
    break;
  case BLE_GATTS_EVT_HVN_TX_COMPLETE:
    NRF_LOG_INFO("Not implemented: %d", (int) p_ble_evt -> header.evt_id);
    break;
  default:
    NRF_LOG_INFO("Not implemented: %d", (int) p_ble_evt -> header.evt_id);

    break;
  }
}

static void gatt_init(void) {
  ret_code_t err_code = nrf_ble_gatt_init( & m_gatt, NULL);
  APP_ERROR_CHECK(err_code);
}
/**@brief Function for initializing the BLE stack.
 *
 * @details Initializes the SoftDevice and the BLE event interrupt.
 */
static void ble_stack_init(void) {
  ret_code_t err_code;
  err_code = nrf_sdh_enable_request();
  APP_ERROR_CHECK(err_code);

  // Configure the BLE stack using the default settings.
  // Fetch the start address of the application RAM.
  uint32_t ram_start = 0;
  err_code = nrf_sdh_ble_default_cfg_set(APP_BLE_CONN_CFG_TAG, & ram_start);
  APP_ERROR_CHECK(err_code);
  // Enable BLE stack.
  err_code = nrf_sdh_ble_enable( & ram_start);
  APP_ERROR_CHECK(err_code);

  // Register a handler for BLE events.
  NRF_SDH_BLE_OBSERVER(m_ble_observer, APP_BLE_OBSERVER_PRIO, ble_evt_handler, NULL);
  NRF_LOG_INFO("obs.");
  NRF_LOG_FLUSH();
}


uint32_t send_stream_pack(uint16_t conn_handle, int16_t AccValue[STREAM_SIZE]) {
  ble_gatts_hvx_params_t params;
  uint16_t len = sizeof(int16_t) * STREAM_SIZE;
  memset( & params, 0, sizeof(params));
  params.type = BLE_GATT_HVX_NOTIFICATION;
  params.handle = m_our_service.mems_handle.value_handle;
  params.p_data = (uint8_t * ) AccValue;
  params.p_len = & len;
  return sd_ble_gatts_hvx(conn_handle, & params);
}

static void app_timer_handler(void * p_context) {

  if (!started_everything || started_timer <= 0) {
    started_timer--;
    if (started_timer <= 0) {
      sleep_mode_enter();
    }
    return;
  }
  started_timer++;

  if (m_our_service.conn_handle != BLE_CONN_HANDLE_INVALID) {
    if (received_config_integer) {
      if (advCountToSleep % 50 == 0) {
        nrf_gpio_pin_set(config_integer == 0 ? PIN_LED_1 : PIN_LED_2 );
      } else {
        nrf_gpio_pin_clear(config_integer == 0 ? PIN_LED_1 : PIN_LED_2);
      }
    } else {
      if (advCountToSleep % 10 == 0) {
        nrf_gpio_pin_set(cycleTimer);
        canCycle = true;
      } else {
        nrf_gpio_pin_clear(cycleTimer);
        if (canCycle) {
          canCycle = false;
          cycleTimer++;
          if (cycleTimer > PIN_LED_2) {
            cycleTimer = PIN_LED_1;
          }
        }
      }
    }

    advCountToSleep++;

    int16_t x=0;
    int16_t y=0;
    int16_t z=0;
    if (hasLis3) {
      if (!LSM6_readAcceleration( & AccValue[0], & AccValue[1], & AccValue[2])) {
        return;
      }

      if (!LSM6_readGyro( & AccValue[3], & AccValue[4], & AccValue[5])) {
        return;
      }

      if (!LSM6_readTemperature( & AccValue[6])) {
        return;
      }
    }

    uint8_t * aux = (uint8_t * )&AccValue[7];

    aux[0] = config_integer;
    

    for (int i = 0; i < 6; i++) {
      uint32_t state = nrf_gpio_pin_read(button_pins[i]);
      aux[i+1] = !state;
    }


    if (rebeginCycles > 0) {
      rebeginCycles--;
    } else {
      if (received_config_integer) {
        send_stream_pack(m_our_service.conn_handle, AccValue);

        blinkTimer--;
        if (blinkTimer <= 0) {
          blinkTimer = 80;
        }
      }
    }
  } else {
    advCountToSleep++;

    int16_t gx=0;
    int16_t gy=0;
    int16_t gz=0;
    if (hasLis3) {

      LSM6_readGyro( & gx, & gy, & gz);
    }
    if (advCountToSleep < 1995) {
      NRF_LOG_INFO("Count: %d/%d (%d, %d, %d) ", advCountToSleep, started_timer,gx, gy, gz);
    }

    if (advCountToSleep >= 2001) {
      sleep_mode_enter();
    }
  }

 
  started_timer--;
}

static void timers_init(void) {

  ret_code_t err_code = nrf_drv_clock_init();
  APP_ERROR_CHECK(err_code);

  // request the lf clock to not to generate any events on ticks
  // One tick =  1 value increment in the counter register
  nrf_drv_clock_lfclk_request(NULL);

  // Initialize timer module, making it use the scheduler
  err_code = app_timer_init();
  APP_ERROR_CHECK(err_code);

  // Create an application timer with the handle, mode and interrupt event handle function
  err_code = app_timer_create( & m_app_timer_id, APP_TIMER_MODE_REPEATED, app_timer_handler);
  APP_ERROR_CHECK(err_code);
}

static void power_management_init(void) {
  ret_code_t err_code;
  err_code = nrf_pwr_mgmt_init();
  APP_ERROR_CHECK(err_code);
}

/**@brief Function for handling the idle state (main loop).
 *
 * @details If there is no pending log operation, then sleep until next the next event occurs.
 */
static void idle_state_handle(void) {
  if (NRF_LOG_PROCESS() == false) {
    nrf_pwr_mgmt_run();
  }
}

/**@brief Function for application main entry.
 */

static bool isOn(int pin) {
  int cntr = 0;
  int c0 = 0;
  int c1 = 0;
  for (cntr = 0; cntr < 11; cntr++) {
    if (nrf_gpio_pin_read(pin) == 0) {
      c0++;
    } else {
      c1++;
    }
    nrf_delay_ms(5);
  }
  return c1 > c0;
}


int main(void) {
  nrf_gpio_cfg_output(PIN_POWER_ACC);

  nrf_gpio_pin_set(PIN_POWER_ACC);
  started_everything = false;
  started_timer = 100;

  m_our_service.conn_handle = BLE_CONN_HANDLE_INVALID;
  rebeginCycles = 50;
 
  log_init();
  leds_init();
  timers_init();

  memset(button_states, 0, sizeof(uint8_t) * 10);

  nrf_gpio_cfg_output(PIN_LED_1);
  nrf_gpio_cfg_output(PIN_LED_2);

  nrf_gpio_pin_clear(PIN_LED_2);
  nrf_gpio_pin_clear(PIN_LED_1);

   for (int i = 0; i < 6; i++) {
      nrf_gpio_cfg_input(button_pins[i], NRF_GPIO_PIN_PULLUP);
   }

  
  nrf_gpio_pin_set(PIN_LED_2);
  nrf_delay_ms(50);
  nrf_gpio_pin_set(PIN_LED_1);
  nrf_delay_ms(50);


  nrf_gpio_pin_clear(PIN_LED_2);
  nrf_delay_ms(50);
  nrf_gpio_pin_clear(PIN_LED_1);
  nrf_delay_ms(500);

  adv_led = PIN_LED_1;
  gmode = 0;

  loud_led = false;

  twi_master_init();
  NRF_LOG_FLUSH();
  nrf_delay_ms(50);
 
  power_management_init();

  
  nrf_gpio_pin_set(PIN_LED_2);
  nrf_gpio_pin_clear(PIN_LED_1);
  if (LSM6_init() == false) {
    NRF_LOG_INFO("LSM6 initialization failed!!!"); // if it failed to initialize then print a message
    NRF_LOG_FLUSH();

    for (int i = 0; i < 4; i++) {
      
      nrf_gpio_pin_clear(PIN_LED_2);
      nrf_gpio_pin_clear(PIN_LED_1);
      nrf_delay_ms(250);
    
      nrf_gpio_pin_set(PIN_LED_2);
      nrf_gpio_pin_set(PIN_LED_1);
      nrf_delay_ms(250);
    }

    hasLis3 = false;
    NRF_LOG_INFO("LSM6 initialization failed again!!!"); // if it failed to initialize then print a message
    NRF_LOG_FLUSH();
  } else {
    hasLis3 = true;
    LSM6_set_power_mode(true);
    LSM6_configure();
  }
  nrf_gpio_pin_clear(PIN_LED_2);

  ble_stack_init();
  gap_params_init();
  gatt_init();
  services_init();
  advertising_init();
  conn_params_init();

  NRF_LOG_INFO("Proto paw started");

  app_timer_start(m_app_timer_id, LED_INTERVAL, NULL);

  advertising_start();
  NRF_LOG_INFO("Begin!");
  started_everything = true;
  started_timer = 20;
  for (;;) {
    idle_state_handle();
  }
}
