secs=1100
while [ $secs -lt 1120 ]; do
   echo "$secs"
   : $((secs++))
done
echo "{"
secs=1200
while [ $secs -lt 1299 ]; do
   echo "$secs"
   : $((secs++))
done
echo "}"
secs=1300
while [ $secs -lt 1340 ]; do
   echo "$secs"
   : $((secs++))
done