#!/bin/bash

git clone --recursive https://github.com/duytai/sFuzz

apt update
apt-get install libleveldb-dev -y

cd for_sFuzz
cp ContractABI.cpp ContractABI.h Fuzzer.cpp Fuzzer.h TargetExecutive.cpp TargetExecutive.h ../sFuzz/libfuzzer/
cp main.cpp Utils.h ../sFuzz/fuzzer

cd ../sFuzz
sh scripts/install_deps.sh

mkdir build
cd build
cmake ..
cd fuzzer
make -j 4

if [ $? -eq 0 ]; then
    echo "\\n******************************************************\\n"
    echo "##########  Initial sfuzz Successfully!  ###########  "
    echo "\\n******************************************************\\n"
else
    echo "Failed to initialize the fuzzer ... ... ... "
fi

cp fuzzer ../../../../test_field/EchoFuzzer

if [ $? -eq 0 ]; then
    echo "\\n******************************************************\\n"
    echo "#@@@@@  Initial EchoFuzz Successfully!  @@@@@#  "
    echo "\\n******************************************************\\n"
else
    echo "Failed to initialize the EchoFuzzer ... ... ... "
fi