secs=2100
while [ $secs -lt 2120 ]; do
   echo "$secs"
   : $((secs++))
done
echo "{"
secs=2200
while [ $secs -lt 2299 ]; do
   echo "$secs"
   : $((secs++))
done
echo "}"
secs=2300
while [ $secs -lt 2340 ]; do
   echo "$secs"
   : $((secs++))
done