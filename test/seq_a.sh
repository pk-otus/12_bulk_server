secs=0
while [ $secs -lt 100 ]; do
   echo "$secs"
   sleep 0.1
   : $((secs++))
done