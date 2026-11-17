#!/bin/bash

if [[ $# -ne 3 ]]; then
    echo "NO of arguments dont match! "
else 
    principal=$1
    amount=$2
    rate=$3
    result=$(( principal * amount * rate / 100 ))
    total=$(( result + principal ))
    echo "Simple Interest = $result and total amount = $total"

fi