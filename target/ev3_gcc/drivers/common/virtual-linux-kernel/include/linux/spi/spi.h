/**
 * Override <linux/spi/spi.h>
 */

struct spi_device {
	void *drvdata;
};

struct spi_driver {
    const struct spi_device_id *id_table;
    int         (*probe)(struct spi_device *spi);
    int         (*remove)(struct spi_device *spi);
    struct device_driver    driver;
};

static inline void spi_set_drvdata(struct spi_device *spi, void *data) {
	spi->drvdata = data;
}
