#!/bin/bash

current_exec_file=${1:-None}

echo "current_exec_file is: $current_exec_file"

cd ../for_IR-Fuzz
cp ContractABI.cpp ContractABI.h Fuzzer.cpp Fuzzer.h TargetExecutive.cpp TargetExecutive.h ../$current_exec_file/sFuzz/libfuzzer/
cp main.cpp Utils.h ../$current_exec_file/sFuzz/fuzzer

cd ../$current_exec_file
rm -rf sFuzz/build/*

./initial_.sh

#unzip ../vfcs_dataset_0_4.zip
#echo "Done!"
