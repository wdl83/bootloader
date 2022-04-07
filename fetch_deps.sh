# !/bin/bash

DRV_DIR=atmega328p_drv
MODBUS_C=modbus_c

if [ ! -d ${DRV_DIR} ]
then
    git clone https://github.com/wdl83/atmega328p_drv ${DRV_DIR}
fi

if [ ! -d ${MODBUS_C} ]
then
    git clone https://github.com/wdl83/modbus_c ${MODBUS_C}
fi
