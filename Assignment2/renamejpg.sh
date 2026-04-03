#!/bin/bash

date=$(date +%F)

for file in *.jpg
do
    if [ -f "$file" ]; then
        mv "$file" "$date-$file"
    fi
done

echo "All JPG files renamed with date prefix"

