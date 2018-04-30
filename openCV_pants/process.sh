#!/bin/bash
for file in /pants/JPGS/*
do
    echo "processing $file"
    ./main "$file" >> OGR_measurements.csv
done
echo "done"
# ls -1 /pants/JPGS | xargs -d'\n' ../main >> ../OGR_measurements.csv