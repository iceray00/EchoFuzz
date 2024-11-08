#!/bin/bash
CURRENT_INDEX=0
START_INDEX=${1:-0}
DURATION=${2:-5}
STOP_INDEX=${3:-99999}

echo "current index is: $START_INDEX"

sh rm.sh

for sol in FNT_dataset/*.sol; do
  echo "================================================="
  echo "------ Current solidity file: $sol ------"

  if [ "$CURRENT_INDEX" -lt "$START_INDEX" ]; then
    CURRENT_INDEX=$((CURRENT_INDEX + 1))
    continue
  fi

  if [ "$CURRENT_INDEX" -ge "$STOP_INDEX" ]; then
    echo "Reached the stop index: $STOP_INDEX. Exiting loop."
    break
  fi

  echo "=====********* Processing file $((CURRENT_INDEX + 1)) *********====="
  cp "$sol" source_code/
  sh rename_src.sh
  sh run_Mu_FNT.sh "$DURATION"
#  contractName="$(basename "$sol" .sol)"
  rm totalBranchSnippet_*.txt sequence_*.txt coverage_*.txt covered_*.txt tractbits_*.txt
#  python3 tempToJson_IR-Fuzz.py --sol_name $contractName --model_name MuFUZZZZ -d $DURATION -g 1
  rm temp_*.txt
#  python3 generate_summary_csv.py --sol_name $contractName --model_name MuFUZZZZ --duration $DURATION --gap_time 1
  sh rm.sh
  sleep 5
  echo "=== Sleep wait 5s! ==="

  # Increment CURRENT_INDEX after processing
  CURRENT_INDEX=$((CURRENT_INDEX + 1))
done
