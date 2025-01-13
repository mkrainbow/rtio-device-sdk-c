#!/bin/bash  

# Copyright (c) 2024-2025 mkrainbow.com.
#
# Licensed under MIT.
# See the LICENSE for detail or copy at https://opensource.org/license/MIT.

if [ "$#" -ne 1 ]; then  
    echo "Usage: $0 <number_of_iterations>"  
    exit 1  
fi  
  
ITERATIONS=$(($1)) 

for ((i=1; i<=$ITERATIONS; i++))  
do  
    echo -n "Iteration $i/$ITERATIONS "  

    dataFileName=2k_uri_$i.data 
    ./crc_test > $dataFileName
    python3 ./crc_check_data.py $dataFileName
done 

  
echo "All iterations completed."



