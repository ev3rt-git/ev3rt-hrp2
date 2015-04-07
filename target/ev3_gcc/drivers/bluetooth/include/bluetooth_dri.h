#pragma once

extern void initialize_bluetooth_dri(intptr_t exinf);
extern void bluetooth_task(intptr_t exinf);
extern void bluetooth_uart_isr();
extern void bluetooth_qos_task(intptr_t exinf);
