#!/bin/bash

base_dir=test_data
input_path="${base_dir}/input"
result_path="${base_dir}/output"
output_dir="build"

echo "Input ${input_path}"
echo "Output ${result_path}"

mkdir -p $output_dir
rm $output_dir/*

make

echo -e "\nStarting test\n\n"

for f in ${input_path}/**/*
do
    ./uvcompress < $f | compress -d | diff $f - > /dev/null 2>&1    
    
    if [ $? -ne 0 ]
    then
        echo -e "Bad diff for $f\n"
    fi
 done

