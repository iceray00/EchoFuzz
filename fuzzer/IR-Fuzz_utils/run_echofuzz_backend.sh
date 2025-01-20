#!/bin/bash

print_usage_and_exit() {
    echo "Welcome to EchoFuzz!!"
    echo "Usage: $0 [DURATION] [ROUND] [MODEL_NAME] [PROMPT_LANGUAGE]"
    echo "Options:"
    echo "  DURATION:    The duration value (default: 60). It represents the time period (in seconds or other relevant unit depending on the context) for which the operation related to the model should run."
    echo "  ROUND:       The round value (default: 3). This might be related to the number of iterations or rounds of processing that the script will perform with the model."
    echo "  MODEL_NAME:  The model name value (default: gpt-4o-mini). Specifies the name of the model that the script will interact with."
    echo "  PROMPT_LANGUAGE: The language prompt value (default: 'cn'). It indicates the language in which the prompts or input to the model should be provided. For example, if set to 'en', the prompts should be in English. Other valid language codes can be used depending on the supported languages of the model."
    echo "  -h, --help:  Display this help message"
    echo "Please make sure to provide the correct parameters according to your needs."
    exit 1
}

if [ "$1" == "--help" ] || [ "$1" == "-h" ]; then
    print_usage_and_exit
fi

rm swap_backend/*
rm swap_backend/fuzz_state/*

echo "0" > swap_backend/signal_iteration.txt

# 0 没开始iteration (i.e., 正在 Chain-guided LLM)
# 1 正在进行iteration_LLM
# 2 正在进行iteration_fuzzing
# 5 正在进行Chain-guided后的fuzzing


DURATION=${1:-60}
ROUND=${2:-3}
MODEL_NAME=${3:-"gpt-4o-mini"}
PROMPT_LANGUAGE=${4:-"cn"}

echo "Language Prompt: $PROMPT_LANGUAGE"
export LANGUAGE_PROMPT=$PROMPT_LANGUAGE
#env | grep LANGUAGE_PROMPT
echo "set LANGUAGE_PROMPT successfully!"
export DB_API_KEY=sk-jL5y8cDbkzMVyTosKnkdG861AwAHct5d69gookDCvEdfgK80
echo "set DB_API_KEY successfully!"

sh rename_src.sh

d1=`ls ./contracts/`

contractName="$(basename "$d1" .sol)"
solc --combined-json abi,bin,bin-runtime,srcmap,srcmap-runtime,ast ./contracts/${contractName}/${contractName}.sol > ./contracts/${contractName}/${contractName}.sol.json

for ((i_pro=1; i_pro<=ROUND ; i_pro++)); do  # Start EchoFuzz
  echo -e "\n********============== file $((CURRENT_INDEX + 1)),  round: $i_pro ==============********\n"

  # for the first time, Chain-guided LLM
  if [ $i_pro -eq 1 ]; then
    echo -e "\n@@@@ Start Chain-Guided LLM to Generate candidate VFCS! @@@@\n"
    # start Chain-guided LLM Generating VFCS!
    python3 EchoFuzz_utils/chain_guided.py -i contracts/${contractName}/${contractName}.sol --model_name $MODEL_NAME
    # will output the result of Chain-guided VFCS in "chain_guided_seed_pool/${MODEL_NAME}/" dir
    python3 EchoFuzz_utils/replace_abi.py --input chain_guided_seed_pool/${MODEL_NAME}/${contractName}_VFCS_1.json --target_dir contracts/${contractName} --id 1
    echo "5" > swap_backend/signal_iteration.txt  # 5 正在进行Chain-guided后的fuzzing
    ./fuzz --file contracts/${contractName}/${contractName}.sol.json --source contracts/${contractName}/${contractName}.sol --name $contractName --assets assets --duration "$DURATION" --mode 0 --reporter 0 --is_first 1 --prefuzz
    if [ $? -eq 1 ]; then
      python3 EchoFuzz_utils/replace_abi.py --input chain_guided_seed_pool/${MODEL_NAME}/${contractName}_VFCS_2.json --target_dir contracts/${contractName} --id 2
      ./fuzz --file contracts/${contractName}/${contractName}.sol.json --source contracts/${contractName}/${contractName}.sol --name $contractName --assets assets --duration "$DURATION" --mode 0 --reporter 0 --is_first 1 --prefuzz
      if [ $? -eq 1 ]; then
        python3 EchoFuzz_utils/replace_abi.py --input chain_guided_seed_pool/${MODEL_NAME}/${contractName}_VFCS_3.json --target_dir contracts/${contractName} --id 3
        ./fuzz --file contracts/${contractName}/${contractName}.sol.json --source contracts/${contractName}/${contractName}.sol --name $contractName --assets assets --duration "$DURATION" --mode 0 --reporter 0 --is_first 1 --prefuzz
      fi
    fi

    d1=`ls ./contracts/`
    for i in $d1
    do
      if [ -d "./contracts/$i" ];then
        d2=`ls ./contracts/$i`
        for j in $d2
        do
          if [ "${j##*.}"x = "sol"x ];then
            eval "solc --bin-runtime --overwrite ./contracts/${i}/${j} -o ./contracts/${i}/"
            name=$(basename $j .sol)
            if [ ! -f $name".bin-runtime" ];then
              echo $name
              eval "evm disasm ./contracts/${i}/$name.bin-runtime |tail -n +2 > ./contracts/${i}/$name.asm"
            fi
          fi
        done
      fi
    done
    cd tools/
    python3 get_targetLoc.py
    cd ..
    ./analyse_prefix > logs/analyze.txt
    python3 tools/gen_attack_contract.py contracts/${contractName}/${contractName}.sol
    ./fuzz --file contracts/${contractName}/${contractName}.sol.json --source contracts/${contractName}/${contractName}.sol --name $contractName --assets assets --duration "$DURATION" --mode 0 --reporter 2 --attacker ReentrancyAttacker --is_first 0 --testcases 5
    if [ $? -eq 0 ]; then
      echo " === round: $i_pro successfully! === "
    else
      echo " === round: $i_pro failed! === "
    fi

  # /* ====== for the other time, employing LLM-Guided Iterative Fuzzing Process to Generate candidate VFCS ====== */
  else
    echo "2" > swap_backend/signal_iteration.txt  # 2 正在进行iteration_fuzzing
    ./fuzz --file contracts/${contractName}/${contractName}.sol.json --source contracts/${contractName}/${contractName}.sol --name $contractName --assets assets --duration "$DURATION" --mode 0 --reporter 0 --is_first 0 --prefuzz
    # if there is a problem with the first VFCS that is replaced and it does not run, the second VFCS is replaced and run
    if [ $? -eq 1 ]; then
      python3 EchoFuzz_utils/replace_abi.py --input fuzzer_pro_vfcs/${contractName}_VFCS_2.json --target_dir contracts/${contractName} --id 2
      ./fuzz --file contracts/${contractName}/${contractName}.sol.json --source contracts/${contractName}/${contractName}.sol --name $contractName --assets assets --duration "$DURATION" --mode 0 --reporter 0 --is_first 0 --prefuzz
    fi
    d1=`ls ./contracts/`
    for i in $d1
    do
      if [ -d "./contracts/$i" ];then
        d2=`ls ./contracts/$i`
        for j in $d2
        do
          if [ "${j##*.}"x = "sol"x ];then
            eval "solc --bin-runtime --overwrite ./contracts/${i}/${j} -o ./contracts/${i}/"
            name=$(basename $j .sol)
            if [ ! -f $name".bin-runtime" ];then
              echo $name
              eval "evm disasm ./contracts/${i}/$name.bin-runtime |tail -n +2 > ./contracts/${i}/$name.asm"
            fi
          fi
        done
      fi
    done
    cd tools/
    python3 get_targetLoc.py
    cd ..
    ./analyse_prefix > logs/analyze.txt
    python3 tools/gen_attack_contract.py contracts/${contractName}/${contractName}.sol
    ./fuzz --file contracts/${contractName}/${contractName}.sol.json --source contracts/${contractName}/${contractName}.sol --name $contractName --assets assets --duration "$DURATION" --mode 0 --reporter 2 --attacker ReentrancyAttacker --is_first 0 --testcases 5
    if [ $? -eq 0 ]; then
      echo " === round: $i_pro successfully! === "
    else
      echo " === round: $i_pro failed! === "
    fi
  fi
  if [ $i_pro -eq "$ROUND" ]; then  # By the last round of execution, there is no need to invoke LLM to generate new VFCS
    continue
  fi

  echo "1" > swap_backend/signal_iteration.txt  # 1 正在进行iteration_LLM
  # LLM-guided Iterative Fuzzing generating VFCS-Next
  python3 EchoFuzz_utils/iteration_process.py -i contracts/${contractName}/${contractName}.sol -d "$DURATION" -m "$MODEL_NAME"

  python3 EchoFuzz_utils/replace_abi.py --input fuzzer_pro_vfcs/${contractName}_VFCS_1.json --target_dir contracts/${contractName} --id 1
  if [ $? -eq 0 ]; then
    echo "Execution successful!"
  fi
done

sh clearTemp.sh

rm swap_backend/*
rm swap_backend/fuzz_state/*