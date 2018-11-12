secs=100
while [ $secs -lt 200 ]; do
   echo "$secs"
   sleep 0.1
   : $((secs++))
done