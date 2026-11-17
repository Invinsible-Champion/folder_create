#!/bin/bash

is_palindrome(){
    local input=$1
    local len=${#input}
    local reversed=""
    for (( i=len-1; i>=0; i--)); do
        reversed="${reversed}${input:$i:1}"
    done
    if [[ "$input" == "$reversed" ]]; then
        return 0
    else 
        return 1
    fi
}

read -p "Enter a value" val

if is_palindrome "$val"; then 
    echo "palindrome"
else 
    echo "not palindrome"
fi


