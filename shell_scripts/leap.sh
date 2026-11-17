#!/bin/bash
read -p "Enter a year: " year

if [[ $(( year % 400 )) -eq 0 || ( $(( year % 4 )) -eq 0 && $(( year % 100 )) -ne 0 ) ]]; then
    echo "$year is a leap year"
else 
    echo "$year is not a leap year"
fi

read -p "Enter a number: " num

if [[ $(( num % 2 )) -eq 0 ]]; then
    echo "The number is even"
else 
    echo "The number is odd"
fi

if [[ $num -gt 0 ]]; then
    echo "The num is pos"
elif [[ $num -eq 0 ]]; then
    echo "The num is zero"
else    
    echo "The num is neg"
fi