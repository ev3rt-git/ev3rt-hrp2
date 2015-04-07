/**
 * Override <linux/device.h>
 */

#pragma once

struct device_driver {
    const char      *name;
    struct module   *owner;
};
