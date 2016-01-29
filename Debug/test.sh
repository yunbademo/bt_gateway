#! /bin/sh

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/liubo/work/yunba-c-sdk/build/output

./bt_gateway led --appkey 5697113d4407a3cd028abead --deviceid "pc_devid" --bt_dev /dev/rfcomm0
