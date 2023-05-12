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
echo "nfp -m port -e show channel stats | grep -E 'UniCastPkts|port.ch'"
echo 'nfp-reg mecsr:i32.me0.TimestampLow. mecsr:i32.me0.TimestampHgh.'
echo ''

