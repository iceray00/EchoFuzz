#!/bin/bash

cd IR-Fuzz_utils/for_IRfuzz
cp ContractABI.cpp ContractABI.h Fuzzer.cpp Fuzzer.h TargetExecutive.cpp TargetExecutive.h ../../IR-Fuzz/sFuzz/libfuzzer/
cp main.cpp Utils.h ../../IR-Fuzz/sFuzz/fuzzer/
if [ $? -eq 0 ]; then
  echo "change the code to _libfuzzer_ and _fuzzer_ successfully!"
else
  echo "change the code to _libfuzzer_ and _fuzzer_ failed!"
  exit 1
fi

cd ../..
mv IR-Fuzz echoFuzz_field
cd echoFuzz_field
mkdir source_code contracts logs branch_msg sFuzz/build
cd sFuzz/build/
cmake ..
cd fuzzer/
make -j 4
cp fuzzer ../../../fuzz
cd ../../../bran/
go build -v -o ../analyse_prefix
cd ..
cd source_code
mv example1.sol GuessNum.sol
cd ..
# in echoFuzz_field

cd ../IR-Fuzz_utils
cp clearTemp.sh rm.sh run_echofuzz.sh ../echoFuzz_field
cp -r EchoFuzz_utils ../echoFuzz_field

cd ../echoFuzz_field
chmod +x clearTemp.sh rm.sh run_echofuzz.sh
if [ $? -eq 0 ]; then
  echo "Initial EchoFuzz successfully!"
else
  echo "Initial EchoFuzz failed ..."
  exit 1
fi
