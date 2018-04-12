#!/bin/sh
if [ -z "$1" ]; then
	#ip=$(cat /usr/local/etc/rfsom-box-gui/modem-ip)
	ip="192.168.23.1"
else
	ip=$1
fi
#if [ -n "$2" ]; then
#	echo 900000000 > /sys/bus/iio/devices/iio\:device1/out_altvoltage0_RX_LO_frequency
#	echo 950000000 > /sys/bus/iio/devices/iio\:device1/out_altvoltage1_TX_LO_frequency
#else
#	echo 950000000 > /sys/bus/iio/devices/iio\:device1/out_altvoltage0_RX_LO_frequency
#	echo 900000000 > /sys/bus/iio/devices/iio\:device1/out_altvoltage1_TX_LO_frequency
#fi
cat /sys/bus/iio/devices/iio\:device1/out_altvoltage0_RX_LO_frequency
cat /sys/bus/iio/devices/iio\:device1/out_altvoltage1_TX_LO_frequency

subnet=$(cat /usr/local/etc/rfsom-box-gui/modem-subnet)
#delay=$(cat /usr/local/etc/rfsom-box-gui/modem-delay)
delay=100000
echo $ip
echo $subnet
echo $delay

devnr=$(cat /sys/bus/iio/devices/iio\:device*/name | grep cf-ad9361-lpc | wc -l)
if [ $devnr -eq 0 ]
then
        echo cf-ad9361-lpc device not found
fi

PID=$(pidof modemd)
if [ -n "$PID" ]; then
	kill -9 $PID
fi

# en_radio.sh
echo 20000000 > /sys/bus/iio/devices/iio:device1/in_voltage_sampling_frequency
cat /sys/bus/iio/devices/iio:device1/in_voltage_sampling_frequency

echo 20000000 > /sys/bus/iio/devices/iio:device1/out_voltage_sampling_frequency
cat /sys/bus/iio/devices/iio:device1/out_voltage_sampling_frequency

echo 20000000 > /sys/bus/iio/devices/iio:device1/in_voltage_rf_bandwidth
echo 20000000 > /sys/bus/iio/devices/iio:device1/out_voltage_rf_bandwidth
cat /sys/bus/iio/devices/iio:device1/in_voltage_rf_bandwidth
cat /sys/bus/iio/devices/iio:device1/out_voltage_rf_bandwidth

cat /sys/bus/iio/devices/iio:device1/in_voltage0_rssi
# en_dds.sh
echo 0x40 0x0 > /sys/kernel/debug/iio/iio:device3/direct_reg_access
cat /sys/kernel/debug/iio/iio:device3/direct_reg_access
echo 0x40 0x2 > /sys/kernel/debug/iio/iio:device3/direct_reg_access
cat /sys/kernel/debug/iio/iio:device3/direct_reg_access
echo 0x40 0x3 > /sys/kernel/debug/iio/iio:device3/direct_reg_access
cat /sys/kernel/debug/iio/iio:device3/direct_reg_access

echo 0x4C 0x3 > /sys/kernel/debug/iio/iio:device3/direct_reg_access
cat /sys/kernel/debug/iio/iio:device3/direct_reg_access

echo 0x44 0x0 > /sys/kernel/debug/iio/iio:device3/direct_reg_access
cat /sys/kernel/debug/iio/iio:device3/direct_reg_access

echo 0x48 0x0 > /sys/kernel/debug/iio/iio:device3/direct_reg_access
cat /sys/kernel/debug/iio/iio:device3/direct_reg_access
for i in $(seq 0 3)
do
	echo $((0x418 + $i * 0x40)) 0 > /sys/kernel/debug/iio/iio:device3/direct_reg_access
	cat /sys/kernel/debug/iio/iio:device3/direct_reg_access
done

echo 0x44 0x1 > /sys/kernel/debug/iio/iio:device3/direct_reg_access
cat /sys/kernel/debug/iio/iio:device3/direct_reg_access
# en_dma.sh
echo 0x40 0x0 > /sys/kernel/debug/iio/iio:device3/direct_reg_access
cat /sys/kernel/debug/iio/iio:device3/direct_reg_access
echo 0x40 0x2 > /sys/kernel/debug/iio/iio:device3/direct_reg_access
cat /sys/kernel/debug/iio/iio:device3/direct_reg_access
echo 0x40 0x3 > /sys/kernel/debug/iio/iio:device3/direct_reg_access
cat /sys/kernel/debug/iio/iio:device3/direct_reg_access

echo 0x4C 0x3 > /sys/kernel/debug/iio/iio:device3/direct_reg_access
cat /sys/kernel/debug/iio/iio:device3/direct_reg_access

echo 0x44 0x0 > /sys/kernel/debug/iio/iio:device3/direct_reg_access
cat /sys/kernel/debug/iio/iio:device3/direct_reg_access

echo 0x48 0x0 > /sys/kernel/debug/iio/iio:device3/direct_reg_access
cat /sys/kernel/debug/iio/iio:device3/direct_reg_access
for i in $(seq 0 3)
do
	echo $((0x418 + $i * 0x40)) 2 > /sys/kernel/debug/iio/iio:device3/direct_reg_access
	cat /sys/kernel/debug/iio/iio:device3/direct_reg_access
done

echo 0x44 0x1 > /sys/kernel/debug/iio/iio:device3/direct_reg_access
cat /sys/kernel/debug/iio/iio:device3/direct_reg_access

#load a custom FIR
echo 0 > /sys/bus/iio/devices/iio:device1/in_voltage_filter_fir_en
cat /usr/local/share/rfsom-box-gui/modem_filter.ftr > /sys/bus/iio/devices/iio:device1/filter_fir_config
echo 1 > /sys/bus/iio/devices/iio:device1/in_voltage_filter_fir_en

#start modem daemon
./ip_reg -x
./modemd -a $ip -m $subnet -d $delay -n tap 
#/usr/local/bin/en_macsec.sh

