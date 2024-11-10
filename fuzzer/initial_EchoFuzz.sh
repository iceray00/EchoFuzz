#!/bin/bash

sudo apt update
sudo apt install -y cmake golang-go libleveldb-dev libcrypto++-dev
export GOPROXY=https://goproxy.io
git clone https://github.com/Messi-Q/IR-Fuzz


cd IR-Fuzz_utils/for_IRfuzz
cp ContractABI.cpp ContractABI.h Fuzzer.cpp Fuzzer.h TargetExecutive.cpp TargetExecutive.h ../../IR-Fuzz/sFuzz/libfuzzer/
cp main.cpp Utils.h ../../IR-Fuzz/sFuzz/fuzzer/
echo 'change the code to _libfuzzer_ and _fuzzer_ successfully!'

cd ../..
mv IR-Fuzz EchoFuzz_field
cd EchoFuzz_field
mkdir source_code contracts logs branch_msg sFuzz/build
cd sFuzz/build/
cmake ..
cd fuzzer/
make -j 4
cp fuzzer ../../../fuzz
cd ../../../bran/
go build -v -o ../analyse_prefix
cd ..
# in EchoFuzz_field

cd ../IR-Fuzz_utils
cp clearTemp.sh rm.sh run_echofuzz.sh ../EchoFuzz_field
cp -r EchoFuzz_utils ../EchoFuzz_field

cd ../EchoFuzz_field
chmod +x clearTemp.sh rm.sh run_echofuzz.sh

