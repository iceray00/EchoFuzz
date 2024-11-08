#pragma once
#include <vector>
#include <map>
#include <liboracle/OracleFactory.h>
#include "Common.h"
#include "TargetProgram.h"
#include "ContractABI.h"
#include "TargetContainerResult.h"
#include "Util.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

using namespace dev;
using namespace eth;
using namespace std;
namespace pt = boost::property_tree;

namespace fuzzer {
  struct RecordParam {
    u64 lastpc = 0;
    bool isDeployment = false;
  };
  class TargetExecutive {
      int flag_show = 0;

      TargetProgram *program;
      OracleFactory *oracleFactory;
      ContractABI ca;
      bytes code;

      /* new */
//      size_t specialState;  // 用于保存execSpecial的状态
      bool hasSpecialState; // 指示是否有execSpecial的状态

    public:
      Address addr;
      TargetExecutive(OracleFactory *oracleFactory, TargetProgram *program, Address addr, ContractABI ca, bytes code) {
        this->code = code;
        this->ca = ca;
        this->addr = addr;
        this->program = program;
        this->oracleFactory = oracleFactory;
      }
      TargetContainerResult exec(bytes data, const tuple<unordered_set<uint64_t>, unordered_set<uint64_t>> &validJumpis);
      void deploy(bytes data, OnOpFunc onOp);

      /* new */
      TargetContainerResult execSpecial(const string& jsonData, const tuple<unordered_set<uint64_t>, unordered_set<uint64_t>>& validJumpis);
      vector<TypeDef> parseParams(const pt::ptree& params);
      bytes unpackBytes(const string& data);
  };
}
