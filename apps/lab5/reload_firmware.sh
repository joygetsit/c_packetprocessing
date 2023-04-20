#/bin/bash


./init/wire.sh stop
make clean
make
sleep 1
./init/wire.sh start wire.fw

echo 'nfp-rtsym _counters'
echo 'nfp-rtsym i32._stats'
echo 'nfp-rtsym i33._stats'
echo 'nfp-rtsym _customData'

