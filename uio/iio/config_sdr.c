#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <signal.h>
#include <stdio.h>

#ifdef __APPLE__
#include <iio/iio.h>
#else
#include <iio.h>
#endif

int main(void)
{

	struct iio_context *ctx;
	struct iio_device *dev;
	struct iio_channel *chn;

	ctx = iio_create_default_context();
	dev = iio_context_find_device(ctx, "ad9361-phy");

	// RX
	chn = iio_device_find_channel(dev, "voltage0", false);
	iio_channel_attr_write_longlong(chn, "sampling_frequency", 20000000);
	iio_channel_attr_write_longlong(chn, "rf_bandwidth", 20000000);
	chn = iio_device_find_channel(dev, "altvoltage0", true);
	iio_channel_attr_write_longlong(chn, "frequency", 1900000000);

	// TX
	chn = iio_device_find_channel(dev, "voltage0", true);
	iio_channel_attr_write_longlong(chn, "sampling_frequency", 20000000);
	iio_channel_attr_write_longlong(chn, "rf_bandwidth", 20000000);
	chn = iio_device_find_channel(dev, "altvoltage1", true);
	iio_channel_attr_write_longlong(chn, "frequency", 1900000000);


	iio_context_destroy(ctx);

	return 0;
}
