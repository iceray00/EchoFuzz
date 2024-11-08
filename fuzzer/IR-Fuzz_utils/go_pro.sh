#!/bin/bash
#  ./go_pro.sh 0 10 2 gpt-3.5-turbo
CURRENT_INDEX=0
START_INDEX=${1:-0}
DURATION=${2:-5}
ROUND=${3:-5}
MODEL_NAME=${4:-"gpt-3.5-turbo"}

echo "current index is: $START_INDEX"

sh rm.sh

for sol in vfcs_dataset_0_4/*.sol; do
  echo "================================================="
  echo "------ Current solidity file: $sol ------"
  if [ "$CURRENT_INDEX" -lt "$START_INDEX" ]; then
		CURRENT_INDEX=$((CURRENT_INDEX + 1))
		continue
	fi

	echo "=====********* Processing file $((CURRENT_INDEX + 1)) *********====="
	cp "$sol" source_code/
	sh rename_src.sh
  contractName="$(basename "$sol" .sol)"

  for ((i=1; i<=ROUND ; i++)); do  # Start vLFuzz+IR-Fuzz Pro
    echo -e "\n********============== file $((CURRENT_INDEX + 1)),  round: $i ==============********\n"
    if [ $i -eq 1 ]; then
      ./fuzz -g -p -r 0 -d "$DURATION" --is_first 1 && chmod +x fuzzMe && ./fuzzMe
      if [ $? -eq 1 ]; then
        break
      fi
    else
#      ./fuzz -g -p -r 0 -d "$DURATION" --is_first 0 && chmod +x fuzzMe && ./fuzzMe  # can't execute this directly, in which case overwritten VFCS would be covered by origin sequence from IR-Fuzz
      ./fuzz --file contracts/${contractName}/${contractName}.sol.json --source contracts/${contractName}/${contractName}.sol --name $contractName --assets assets --duration "$DURATION" --mode 0 --reporter 0 --is_first 0 --prefuzz
      if [ $? -eq 0 ]; then
        echo " === round: $i successfully! === "
      else  # If there is a problem with the first VFCS that is replaced and it does not run, the second VFCS is run
        python3 vLFuzz_utils/replace_abi.py --input fuzzer_pro_vfcs/${contractName}_VFCS_2.json --target_dir contracts/${contractName} --id 2
        ./fuzz --file contracts/${contractName}/${contractName}.sol.json --source contracts/${contractName}/${contractName}.sol --name $contractName --assets assets --duration "$DURATION" --mode 0 --reporter 0 --is_first 0 --prefuzz
      fi
    fi
    if [ $i -eq "$ROUND" ]; then  # By the last round of execution, there is no need to invoke LLM to generate new VFCS
      continue
    fi
    # LLM generate the VFCS
    python3 vLFuzz_utils/fuzzer_pro.py -i contracts/${contractName}/${contractName}.sol -d "$DURATION" -m "$MODEL_NAME"
    python3 vLFuzz_utils/replace_abi.py --input fuzzer_pro_vfcs/${contractName}_VFCS_1.json --target_dir contracts/${contractName} --id 1
    if [ $? -eq 0 ]; then
      echo "Execution successful!"
    fi
  done
  python3 vLFuzz_utils/changeTemp.py -i temp_${contractName}_phase1_result.txt --gap_time 1
  rm sequence_${contractName}.txt totalBranchSnippet_${contractName}.txt covered_${contractName}.txt coverage_${contractName}.txt tracebits_${contractName}.txt
  python3 vLFuzz_utils/tempToJson_IR-Fuzz_pro.py --sol_name $contractName --model_name $MODEL_NAME -d $((DURATION * ROUND)) -g 1
  rm temp_${contractName}_phase1_resul*.txt
  python3 vLFuzz_utils/generate_summary_csv2.py --sol_name $contractName --model_name $MODEL_NAME --duration $((DURATION * ROUND)) --gap_time 1
  sh rm.sh

  # Increment CURRENT_INDEX after processing
  CURRENT_INDEX=$((CURRENT_INDEX + 1))
done
