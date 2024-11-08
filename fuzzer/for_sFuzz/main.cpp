#include <iostream>
#include <libfuzzer/Fuzzer.h>
#include "Utils.h"

using namespace std;
using namespace fuzzer;

static int DEFAULT_MODE = AFL;
static int DEFAULT_DURATION = 120; // 2 mins
static int DEFAULT_REPORTER = 2; // BOTH
static int DEFAULT_ANALYZING_INTERVAL = 5; // 5 sec
static string DEFAULT_CONTRACTS_FOLDER = "contracts/";
static string DEFAULT_ASSETS_FOLDER = "assets/";
static string DEFAULT_ATTACKER = "ReentrancyAttacker";
static string DEFAULT_ORDER_SEQUENCE = "ABA"; // ABA | AB | BA
static string DEFAULT_MODEL_NAME = "";
static string DEFAULT_DATASET = "simple_dataset"; // or "vfcs_dataset_0_4"
static int DEFAULT_GAPTIME = 5; // 5 second each save the stat
static int DEFAULT_VFCS_ID = 1;
static string DEFAULT_MER = "avg";
static string DEFAULT_SEED_POOL = "seed_pool";
static int DEFAULT_INDEX = 0;
static int DEFAULT_SEND_TIME = 30; /* phase 2 */

int main(int argc, char* argv[]) {
    /* Run EVM silently */
    dev::LoggingOptions logOptions;
    logOptions.verbosity = VerbositySilent;
    dev::setupLogging(logOptions);
    /* Program options */
    int mode = DEFAULT_MODE;
    int duration = DEFAULT_DURATION;
    int reporter = DEFAULT_REPORTER;
    string contractsFolder = DEFAULT_CONTRACTS_FOLDER;
    string assetsFolder = DEFAULT_ASSETS_FOLDER;
    string jsonFile = "";
    string contractName = "";
    string sourceFile = "";
    string attackerName = DEFAULT_ATTACKER;
    string order_sequence = DEFAULT_ORDER_SEQUENCE;
    string model_name = DEFAULT_MODEL_NAME;
    string dataset = DEFAULT_DATASET;
    int gapTime = DEFAULT_GAPTIME;
    int vfcs_ID = DEFAULT_VFCS_ID;
    string merge = DEFAULT_MER;
    string seedPool = DEFAULT_SEED_POOL;
    int index = DEFAULT_INDEX;
    int sendTime = DEFAULT_SEND_TIME;

    po::options_description desc("Allowed options");
    po::variables_map vm;

    desc.add_options()
        ("help,h", "produce help message")
        ("contracts,c", po::value(&contractsFolder), "contract's folder path")
        ("generate,g", "g fuzzMe script")

        /* add */
        ("vLfuzz,v", "vfcs from LLM")
        ("model_name", po::value(&model_name), "LLM name")
        ("test_seed_pool", "testing seed pool. how many can fuzz!")
        ("dataset", po::value(&dataset), "Data Set")
        ("gap_time,t", po::value(&gapTime), "gap time for each record")
        ("vfcs_id", po::value(&vfcs_ID), "The id of VFCS")
        ("merge", po::value(&merge), "merge method")
        ("seed_pool", po::value(&seedPool), "VFCS seed pool")
        ("index", po::value(&index), "Skip the first index position")
        ("send_time", po::value(&sendTime), "LLM-FUZZER pro! gap time between fuzzer send stat info to LLM")

        ("fuzzer_pro,p", "p Fuzzer Pro!")

        ("assets,a", po::value(&assetsFolder), "asset's folder path")
        ("file,f", po::value(&jsonFile), "fuzz a contract")
        ("name,n", po::value(&contractName), "contract name")
        ("source,s", po::value(&sourceFile), "source file path")
        ("mode,m", po::value(&mode), "choose mode: 0 - AFL ")
        ("reporter,r", po::value(&reporter), "choose reporter: 0 - TERMINAL | 1 - JSON")
        ("duration,d", po::value(&duration), "fuzz duration")
        /* new */
        ("order_seq,or", po::value(&order_sequence), "order sequence: ABA | AB | BA")
        ("attacker", po::value(&attackerName), "choose attacker: NormalAttacker | ReentrancyAttacker");
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);
    /* Show help message */
    if (vm.count("help")) showHelp(desc);


    /* use LLM to generate VFCS. use VFCS to fuzzing! */
    if (vm.count("vLfuzz")) {
      /* Usage:
       *
       * // to test usability
       * ./vLFuzzer -v --merge avg --seed_pool zip_seed_pool -d 1 --gap_time 5 --order_seq ABA --index 0 --dataset vfcs_dataset_0_4 --model_name qwen2_7b && chmod +x ./vLFuzz_pre && ./vLFuzz_pre
       *
       * // to test phase_1 experiment
       * ./vLFuzzer -v -c contract_3.5turbo --merge max --seed_pool seed_pool -d 300 --gap_time 1 --order_seq ABA --index 0 --dataset vfcs_dataset_0_4 --model_name gpt-3.5-turbo && chmod +x ./vLFuzz_pre && ./vLFuzz_pre
       *
       */
        std::ofstream vLFuzz_pre("vLFuzz_pre");
        vLFuzz_pre << "#!/bin/bash" << endl;
        vLFuzz_pre << "SUCCESS_COUNT=0" << endl;
        vLFuzz_pre << "TOTAL_COUNT=0" << endl;
        vLFuzz_pre << "model_name=\"" << model_name << "\"" << endl;
        vLFuzz_pre << "dataset_name=\"" << dataset << "\"" << endl;
        vLFuzz_pre << "seed_pool=\"" << seedPool << "\"" << endl;
        vLFuzz_pre << "duration=\"" << duration << "\"" << endl;
        vLFuzz_pre << "merge=\"" << merge << "\"" << endl;
        vLFuzz_pre << "gap_time=\"" << gapTime << "\"" << endl; /* to save each gap time */
        vLFuzz_pre << "SEED_POOL_DIR=\"../$seed_pool/$model_name\"" << endl;
        vLFuzz_pre << "DATASET_DIR=\"../$dataset_name\"" << endl;
        vLFuzz_pre << "TARGET_DIR=\"" << contractsFolder << "\"" << endl;
        vLFuzz_pre << "START_INDEX=" << index << endl;
        vLFuzz_pre << "CURRENT_INDEX=0" << endl;
        vLFuzz_pre << "contractName=\"\"" << endl;
        vLFuzz_pre << "mkdir -p \"$TARGET_DIR\"" << endl;  // make sure the dir exist
        vLFuzz_pre << "rm -rf contracts/*" << endl;
        vLFuzz_pre << "rm -rf assets/.ipynb_checkpoints" << endl;
        vLFuzz_pre << "for sol in \"$DATASET_DIR\"/*.sol; do" << endl;
        vLFuzz_pre << "   if [ $CURRENT_INDEX -lt $START_INDEX ]; then" << endl;
        vLFuzz_pre << "       CURRENT_INDEX=$((CURRENT_INDEX + 1))" << endl;
        vLFuzz_pre << "       continue" << endl;
        vLFuzz_pre << "   fi" << endl;
        vLFuzz_pre << "   echo \"Processing file $((CURRENT_INDEX + 1))\"" << endl;
        vLFuzz_pre << "   cp \"$sol\" \"$TARGET_DIR\"" << endl;
        vLFuzz_pre << "   echo -e \"\\n\\n************************\\nNow, cp the [[\"$sol\"]] successfully!!!\\n************************\\n \" "<< endl;
        vLFuzz_pre << "   echo -e \" &&&&&&& target_dir is: [[\"$TARGET_DIR\"]]!\\n&&&&&&&&&&&&&&&&&&\\n\" " << endl;
//      vLFuzz_pre << "   " << compileSolFiles(contractsFolder);
        vLFuzz_pre << "   sol_file=\"$TARGET_DIR/$(basename \"$sol\")\"" << endl;
        vLFuzz_pre << "   " << "solc --combined-json abi,bin,bin-runtime,srcmap,srcmap-runtime,ast \"$sol_file\" > \"$sol_file.json\"" << endl;
        vLFuzz_pre << "   " << compileSolFiles(assetsFolder);
//      vLFuzz_pre << "   " << compileSolFiles("$sol_file") << endl;
//      vLFuzz_pre << "   " << compileSolFiles("$TARGET_DIR/$(basename \"$sol\")") << endl;
//      contractName = getContractName(contractsFolder);
//      vLFuzz_pre << "   contractName=\"" << contractName << "\"" << endl;
//      vLFuzz_pre << "   echo \"contractName from main is:\"" << contractName << endl;
        vLFuzz_pre << "   contractName=\"$(basename \"$sol\" .sol)\"" << endl;
        vLFuzz_pre << "   echo -e \"******************contractName from basename is: $contractName \\n================= \"" << endl;
//      vLFuzz_pre << "   for file in \"$SEED_POOL_DIR/${contractName}_VFCS_*.json\"; do" << endl; /* Iterate over the matching files */
        vLFuzz_pre << "   files=$(find \"$SEED_POOL_DIR\" -name \"${contractName}_VFCS_*.json\")" << endl;
        vLFuzz_pre << "   for file in $files; do" << endl;
        vLFuzz_pre << "       echo -e \"Processing file: $file\"" << endl;
        vLFuzz_pre << "       echo -e \" ***^^^^^** Successful enter in Second FOR loop! ****^^^***** \"  " << endl;
//      vLFuzz_pre << "       echo -e \" print the info： \"$SEED_POOL_DIR/${contractName}_VFCS_*.json\" \\n@@@@@@@@@@@@@@@@@@@@@  \" " << endl;
        vLFuzz_pre << "       if [[ -e \"$file\" ]]; then" << endl;
        vLFuzz_pre << "           echo -e \"!!!!!!!!!!!!\\n exist file with {$contractName}_VFCS_*.json!!\\n!!!!!!!!!!  \"  " << endl;
        vLFuzz_pre << "           FILE_ID=$(basename \"$file\" | sed -E \"s/^${contractName}_VFCS_([0-9]+)\\.json$/\\1/\")" << endl;
        vLFuzz_pre << "           python3 ../utils/replace_abi.py --input \"$file\" --target_dir \"$TARGET_DIR\" --id \"$FILE_ID\"" << endl;
//        vLFuzz_pre << "           " << vLfuzzJsonFiles(contractsFolder, assetsFolder, duration, mode, reporter, attackerName, order_sequence) << endl;
//        vLFuzz_pre << "           ./vLFuzzer --file \"$sol_file.json\" --source \"$sol_file\" --name $contractName --assets assets/ --duration " << to_string(duration) << " --mode 0 --reporter 0 --attacker " << attackerName << " --order_seq " << order_sequence << endl;
//        vLFuzz_pre << "           ./vLFuzzer --file \"$sol_file.json\" --source \"$sol_file\" --name $contractName --assets assets/ --duration " << to_string(duration) << " --mode 0 --reporter 0 --attacker " << attackerName << " --order_seq " << order_sequence << " --gap_time $gap_time" << endl;
        vLFuzz_pre << "           ./vLFuzzer --file \"$sol_file.json\" --source \"$sol_file\" --name $contractName --assets assets/ --duration " << to_string(duration) << " --mode 0 --reporter 0 --attacker " << attackerName << " --order_seq " << order_sequence << " --gap_time $gap_time --vfcs_id $FILE_ID" << endl;
        vLFuzz_pre << "           if [ $? -eq 0 ]; then" << endl;
        vLFuzz_pre << "               echo \"Execution successful!\"" << endl;
        vLFuzz_pre << "               SUCCESS_COUNT=$((SUCCESS_COUNT + 1))" << endl;
        vLFuzz_pre << "               TOTAL_COUNT=$((TOTAL_COUNT+1))" << endl;
        vLFuzz_pre << "               echo -e \"\\n\\n ********\\n Current success count: $SUCCESS_COUNT \\n *********\\n\\n \"" << endl;
        vLFuzz_pre << "           else" << endl;
        vLFuzz_pre << "               echo \"Execution failed!\"" << endl;
        vLFuzz_pre << "               TOTAL_COUNT=$((TOTAL_COUNT+1))" << endl;
        vLFuzz_pre << "               echo -e \"\\n\\n ********\\n Current success count: $SUCCESS_COUNT \\n *********\\n\\n \"" << endl;
        vLFuzz_pre << "           fi" << endl;
//        vLFuzz_pre << "           python3 ../utils/saveDataFromTemp.py --sol_name $contractName --model_name $model_name " << endl;
        vLFuzz_pre << "       fi" << endl;
        vLFuzz_pre << "   done" << endl;
        vLFuzz_pre << "   python3 ../utils/tempToJson.py --sol_name $contractName --model_name $model_name -d $duration -g $gap_time -a $merge" << endl;
        vLFuzz_pre << "   rm temp_${contractName}_phase1_result_*.txt" << endl;
//        vLFuzz_pre << "   python3 ../utils/summary_csv.py --sol_name $contractName --model_name $model_name --time " << to_string(duration) << endl;
        vLFuzz_pre << "   python3 ../utils/generate_summary_csv.py --sol_name $contractName --model_name $model_name --duration $duration --gap_time $gap_time" << endl;
        vLFuzz_pre << "   rm -r contracts/*" << endl;
        vLFuzz_pre << "   CURRENT_INDEX=$((CURRENT_INDEX + 1))" << endl;
        vLFuzz_pre << "done" << endl;
        vLFuzz_pre << "echo -e \"\\n\\n\\n ** Using Model: $model_name ** \"  " << endl;
        vLFuzz_pre << "echo -e \"                  ========= Total file count: $TOTAL_COUNT ========== \"  " << endl;
        vLFuzz_pre << "echo -e \"\\n ############### ***** Total successful executions: $SUCCESS_COUNT ***** ############### \" " << endl;
        vLFuzz_pre << "echo \"Total file count: $TOTAL_COUNT\" > results.txt" << endl;
        vLFuzz_pre << "echo \"Total successful executions: $SUCCESS_COUNT\" >> results.txt" << endl;
        vLFuzz_pre << "if [ $TOTAL_COUNT -gt 0 ]; then" << endl;
        vLFuzz_pre << "    SUCCESS_RATIO=$(( SUCCESS_COUNT * 100 / TOTAL_COUNT ))" << endl;
        vLFuzz_pre << "    echo -e \"\\n                  >>>>>>>>>> Success Ratio: $SUCCESS_RATIO% <<<<<<<<<<<\\n\"  " << endl;
        vLFuzz_pre << "    echo \"Success Ratio: $SUCCESS_RATIO%\" >> results.txt" << endl;
        vLFuzz_pre << "else" << endl;
        vLFuzz_pre << "    echo -e \"\\n\\nWARNING/ERROR: >>>>>>>>> No files were processed. <<<<<<<<<<<\\n\" " << endl;
        vLFuzz_pre << "    echo \"WARNING/ERROR: No files were processed.\" >> results.txt" << endl;
        vLFuzz_pre << "fi" << endl;
        vLFuzz_pre.close();
        vLFuzzShowGenerate();
    }

    /* test how much vfcs_abi can fuzz */
    if (vm.count("test_seed_pool")) {
        contractName = getContractName(contractsFolder);
        std::ofstream testSeedPool("testSeedPool");

//        testSeedPool << "#!/bin/bash" << endl;
//        testSeedPool << "SUCCESS_COUNT=0" << endl;
//        testSeedPool << "DATASET_DIR=\"/root/vLFuzz/simple_dataset\"" << endl;
//        testSeedPool << "TARGET_DIR=\"/root/vLFuzz/test_field/contracts\"" << endl;
//        testSeedPool << "mkdir -p \"$TARGET_DIR\"" << endl;  // make sure the dir exist
//        testSeedPool << "for sol in \"$DATASET_DIR\"/*.sol; do" << endl;
//        testSeedPool << "   " << compileSolFiles(contractsFolder);
//        testSeedPool << "   " << compileSolFiles(assetsFolder);
        /*
         * 直接使用-v，然后指定-d 1就能作为测试，不用分开进行了
         * 因此直接修改这个内容：
         *
         */

        testSeedPool << "#!/bin/bash" << endl;
        testSeedPool << "model_name=" << model_name;
        testSeedPool << "./vLFuzzer -v -d 1 --model_name $model_name && chmod +x ./vLFuzz_pre && ./vLFuzz_pre" << endl;
        testSeedPool.close();
        return 0;
    }







    /* Generate working scripts */
    if (vm.count("generate")) {
        std::ofstream fuzzMe("fuzzMe");
        fuzzMe << "#!/bin/bash" << endl;
        fuzzMe << compileSolFiles(contractsFolder);
        fuzzMe << compileSolFiles(assetsFolder);
        //    fuzzMe << fuzzJsonFiles(contractsFolder, assetsFolder, duration, mode, reporter, attackerName);
        fuzzMe << fuzzJsonFiles(contractsFolder, assetsFolder, duration, mode, reporter, attackerName, order_sequence);
        fuzzMe.close();
        showGenerate();
        return 0;
    }

    /* Fuzz a single contract */
    if (vm.count("file") && vm.count("name") && vm.count("source")) {
        FuzzParam fuzzParam;  // the params for fuzzing
        auto contractInfo = parseAssets(assetsFolder);
        contractInfo.push_back(parseSource(sourceFile, jsonFile, contractName, true));
        fuzzParam.contractInfo = contractInfo;
        //    fuzzParam.contractInfo.orderSequence = order_sequence;  /* add ABA or AB or BA order */
        fuzzParam.mode = (FuzzMode) mode;
        fuzzParam.duration = duration;
        fuzzParam.reporter = (Reporter) reporter;
        fuzzParam.analyzingInterval = DEFAULT_ANALYZING_INTERVAL;
        fuzzParam.attackerName = attackerName;
        /* new */
        fuzzParam.orderSeq = order_sequence;  /* add ABA or AB or BA order */
        fuzzParam.gapTime = gapTime;
        fuzzParam.vfcsID = vfcs_ID;
//        fuzzParam.sendTime = sendTime;

        Fuzzer fuzzer(fuzzParam);
        cout << ">> Fuzz " << contractName << endl;
        fuzzer.start();
        return 0;
    }
    return 0;
}
