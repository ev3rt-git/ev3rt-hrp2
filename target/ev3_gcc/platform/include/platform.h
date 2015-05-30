/**
 * EV3RT Platform (Core Services Layer)
 */

typedef struct ev3_driver_information {
	ISR init_func;
	ISR softreset_func;
} ev3_driver_t;

extern ER platform_register_driver(const ev3_driver_t *p_driver);

ER platform_soft_reset();
bool_t platform_is_ready();

/**
 * EV3RT configurations
 */
extern const char   *ev3rt_bluetooth_local_name;   //!< Name for service discovery
extern const char   *ev3rt_bluetooth_pin_code;     //!< Pin code for authentication, up to 16 bytes + '\0'
extern const bool_t *ev3rt_sensor_port_1_disabled; //!< True: use port 1 as a serial port, False: use port 1 as a normal sensor port

/**
 * Tasks
 */
extern void ev3_main_task(intptr_t exinf);
extern void platform_busy_task(intptr_t exinf);

/**
 * Exceptions
 */
extern void ev3_prefetch_handler(void *p_excinf);
extern void ev3_data_abort_handler(void *p_excinf);

