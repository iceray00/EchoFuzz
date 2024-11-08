#include <iostream>
#include <boost/algorithm/string.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <libfuzzer/Fuzzer.h>

using namespace std;
using namespace fuzzer;
using namespace boost::filesystem;
namespace pt = boost::property_tree;
namespace po = boost::program_options;

ContractInfo parseJson(string jsonFile, string contractName, bool isMain) {
  std::ifstream file(jsonFile);
  if (!file.is_open()) {
    stringstream output;
    output << "[x] File " + jsonFile + " is not found" << endl;
    cout << output.str();
    exit(1);
  }
  pt::ptree root;
  pt::read_json(jsonFile, root);
  string fullContractName = "";
  for (auto key : root.get_child("contracts")) {
    if (boost::ends_with(key.first, contractName)) {
      fullContractName = key.first;
      break;
    }
  }
  if (!fullContractName.length()) {
    cout << "[x] No contract " << contractName << endl;
    exit(1);
  }
  pt::ptree::path_type abiPath("contracts|"+ fullContractName +"|abi", '|');
  pt::ptree::path_type binPath("contracts|"+ fullContractName +"|bin", '|');
  pt::ptree::path_type binRuntimePath("contracts|" + fullContractName + "|bin-runtime", '|');
  pt::ptree::path_type srcmapPath("contracts|" + fullContractName + "|srcmap", '|');
  pt::ptree::path_type srcmapRuntimePath("contracts|" + fullContractName + "|srcmap-runtime", '|');
  ContractInfo contractInfo;
  contractInfo.isMain = isMain;
  contractInfo.abiJson = root.get<string>(abiPath);
  contractInfo.bin = root.get<string>(binPath);
  contractInfo.binRuntime = root.get<string>(binRuntimePath);
  contractInfo.srcmap = root.get<string>(srcmapPath);
  contractInfo.srcmapRuntime = root.get<string>(srcmapRuntimePath);
  contractInfo.contractName = fullContractName;
  for (auto it : root.get_child("sources")) {
    auto ast = it.second.get_child("AST");
    vector<pt::ptree> stack = {ast};
    while (stack.size() > 0) {
      auto item = stack[stack.size() - 1];
      stack.pop_back();
      if (item.get<string>("name") == "FunctionDefinition") {
        if (item.get<bool>("attributes.constant")) {
          contractInfo.constantFunctionSrcmap.push_back(item.get<string>("src"));
        }
      }
      if (item.get_child_optional("children")) {
        for (auto it : item.get_child("children")) {
          stack.push_back(it.second);
        }
      }
    }
  }
  return contractInfo;
}

ContractInfo parseSource(string sourceFile, string jsonFile, string contractName, bool isMain) {
  std::ifstream file(sourceFile);
  if (!file.is_open()) {
    stringstream output;
    output << "[xxx iceray xxx] File " + jsonFile + " is not found" << endl;
    cout << output.str();
    exit(1);
  }
  auto contractInfo = parseJson(jsonFile, contractName, isMain);
  std::string sourceContent((std::istreambuf_iterator<char>(file)),(std::istreambuf_iterator<char>()));
  contractInfo.source = sourceContent;
  return contractInfo;
}


string toContractName(directory_entry file) {
  string filePath = file.path().string();
  string fileName = file.path().filename().string();
  string fileNameWithoutExtension = fileName.find(".") != string::npos
  ? fileName.substr(0, fileName.find("."))
  : fileName;
  string contractName = fileNameWithoutExtension.find("_0x") != string::npos
  ? fileNameWithoutExtension.substr(0, fileNameWithoutExtension.find("_0x"))
  : fileNameWithoutExtension;
  return contractName;
}

void forEachFile(string folder, string extension, function<void (directory_entry)> cb) {
  path folderPath(folder);
  for (auto& file : boost::make_iterator_range(directory_iterator(folderPath), {})) {
    if (is_directory(file.status())) forEachFile(file.path().string(), extension, cb);
    if (!is_directory(file.status()) && boost::ends_with(file.path().string(), extension)) cb(file);
  }
}

string compileSolFiles(string folder) {
  stringstream ret;
  forEachFile(folder, ".sol", [&](directory_entry file) {
    string filePath = file.path().string();
    ret << "solc";
    ret << " --combined-json abi,bin,bin-runtime,srcmap,srcmap-runtime,ast " + filePath;
    ret << " > " + filePath + ".json";
    /* Testing */
    cout << "***** Successful exec `compileSolFiles`!" << endl;
    ret << endl;
  });
  return ret.str();
}

//string compileSolFilesICE(string solFilePath) {
//    stringstream ret;
//    ret << "solc --combined-json abi,bin,bin-runtime,srcmap,srcmap-runtime,ast " << solFilePath << " > " << solFilePath << ".json" << endl;
//    return ret.str();
//}



string getContractName(string contracts) {
    string contractName;
    forEachFile(contracts, ".sol", [&](directory_entry file) {
        contractName = toContractName(file);
    });
    return contractName;
}

string fuzzJsonFiles(string contracts, string assets, int duration, int mode, int reporter, string attackerName) {
  stringstream ret;
  unordered_set<string> contractNames;
  /* search for sol file */
  forEachFile(contracts, ".sol", [&](directory_entry file) {
    auto filePath = file.path().string();
    auto contractName = toContractName(file);
    if (contractNames.count(contractName)) return;
    ret << "./fuzzer";
    ret << " --file " + filePath + ".json";
    ret << " --source " + filePath;
    ret << " --name " + contractName;
    ret << " --assets " + assets;
    ret << " --duration " + to_string(duration);
    ret << " --mode " + to_string(mode);
    ret << " --reporter " + to_string(reporter);
    ret << " --attacker " + attackerName;
    /* Testing */
    cout << "***** Successful exec `fuzzJsonFiles`!" << endl;
    ret << endl;
  });
  return ret.str();
}


/* add field Order Sequence. ABA | AB | BA */
string fuzzJsonFiles(string contracts, string assets, int duration, int mode, int reporter, string attackerName, string orderSequence) {
    stringstream ret;
    unordered_set<string> contractNames;
    /* search for sol file */
    forEachFile(contracts, ".sol", [&](directory_entry file) {
        auto filePath = file.path().string();
        auto contractName = toContractName(file);
        if (contractNames.count(contractName)) return;
        ret << "./fuzzer";
        ret << " --file " + filePath + ".json";
        ret << " --source " + filePath;
        ret << " --name " + contractName;
        ret << " --assets " + assets;
        ret << " --duration " + to_string(duration);
        ret << " --mode " + to_string(mode);
        ret << " --reporter " + to_string(reporter);
        ret << " --attacker " + attackerName;
        /* new */
        ret << " --order_seq " + orderSequence;
        /* Testing */
        cout << "***** Successful exec `fuzzJsonFiles`!" << endl;
        ret << endl;
    });
    return ret.str();
}

//string fuzzJsonFilesICE(string solFilePath, string assets, int duration, int mode, int reporter, string attackerName, string orderSequence) {
//    stringstream ret;
//    auto contractName = toContractName(solFilePath);
//    ret << "./fuzzer";
//    ret << " --file " + solFilePath + ".json";
//    ret << " --source " + solFilePath;
//    ret << " --name " + contractName;
//    ret << " --assets " + assets;
//    ret << " --duration " + to_string(duration);
//    ret << " --mode " + to_string(mode);
//    ret << " --reporter " + to_string(reporter);
//    ret << " --attacker " + attackerName;
//    ret << " --order_seq " + orderSequence;
//    cout << "***** Successful exec `fuzzJsonFiles`!" << endl;
//    ret << endl;
//    return ret.str();
//}




string vLfuzzJsonFiles(string contracts, string assets, int duration, int mode, int reporter, string attackerName, string orderSequence) {
    stringstream ret;
    unordered_set<string> contractNames;
    /* search for sol file */
    forEachFile(contracts, ".sol", [&](directory_entry file) {
        auto filePath = file.path().string();
        auto contractName = toContractName(file);
        if (contractNames.count(contractName)) return;
        ret << "./vLFuzzer";
        ret << " --file " + filePath + ".json";
        ret << " --source " + filePath;
        ret << " --name " + contractName;
        ret << " --assets " + assets;
        ret << " --mode " + to_string(mode);
        ret << " --reporter " + to_string(reporter);
        ret << " --attacker " + attackerName;
        /* new */
        ret << " --order_seq " + orderSequence;
        ret << " --duration " + to_string(duration);
        /* Testing */
        cout << "***** Successful exec `fuzzJsonFiles`!" << endl;
        ret << endl;
    });
    return ret.str();
}

           //
//  /* abiJson ------> abiJson */
//  /*        Using LLM        */
//  /* use script to implement */
//void preFuzzingLLM(){
//    /*
//     * params: abiJson
//     *
//     * operate: to change the abiJson file which output from `compileSolFiles`
//     *
//     * and then, `fuzzJsonFiles` use the abiJson after `preFuzzingLLM`
//     */
//
//    // 1. 原本的abi信息
//
//    // 2. 用脚本，调用预设的python脚本
//    //  - 这个python脚本就是调用LLM的脚本，设置好相应的prompt（VTS生成的内容），并利用IR-Fuzz的`gen_func_order.py`文件的执行结果作为一个额外的信息保存到新的json文件中
//    //  - LLM中的prompt接入VFCS的介绍、现在已保存在包中的.sol.json的abi信息、IR-Fuzz得到的函数间调用关系、IR-Fuzz生成的可能序列信息，辅以一定的prompt进行引导
//    //  - 最终：
//    //    - 1、将新给出的abi信息写入回.sol.json中 --overwrite    ::以供sFuzz的fuzzing流程续继续使用！
//    //    - 2、生成一个单独的、以[contract_name_id].vfcs.json显式命名，用于单独存放所有可能的VFCS
//    //    - 3、将所有的打包、写入到新的中，形成seed pool
//
//    // 重新将新的abi信息写入.sol.json中！
//}


vector<ContractInfo> parseAssets(string assets) {
  vector<ContractInfo> ls;
  forEachFile(assets, ".json", [&](directory_entry file) {
    auto contractName = toContractName(file);
    auto jsonFile = file.path().string();
    ls.push_back(parseJson(jsonFile, contractName, false));
  });
  return ls;
}

void showHelp(po::options_description desc) {
  stringstream output;
  output << desc << endl;
  output << "Example:" << endl;
  output << "> Generate executable scripts" << endl;
  output << "  " cGRN "./fuzzer -g" cRST << endl;
  cout << output.str();
}

void showGenerate() {
  stringstream output;
  output << cGRN "> Created \"fuzzMe\"" cRST "\n";
  output << cGRN "> To fuzz contracts:" cRST "\n";
  output << "  chmod +x fuzzMe\n";
  output << "  ./fuzzMe\n";
  cout << output.str();
}

void vLFuzzShowGenerate() {
    stringstream output;
    output << cGRN "> Created \"vLFuzz_pre\"" cRST "\n";
    output << cGRN "> To fuzz contracts:" cRST "\n";
    output << "  chmod +x vLFuzz_pre\n";
    output << "  ./vLFuzz_pre\n";
    output << "Generate `vLFuzz_pre` successfully!\n";

    cout << output.str();
}
