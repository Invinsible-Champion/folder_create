#!/bin/bash
read -p "Enter the value of N " n
x=0
y=1
echo -n "$x "
echo -n "$y "
for (( i=0; i<n-2; i++ )); do
    result=$(( x+y ))
    temp=$y
    x=$temp
    y=$result
    echo -n "$result "
done


for (( i=0; i<=4; i++ )); do
    for (( j=0; j<i; j++)); do 
        echo -n "*"
    done
    echo ""
done