#include "TargetExecutive.h"
#include "Logger.h"
namespace pt = boost::property_tree;

namespace fuzzer {
  void TargetExecutive::deploy(bytes data, OnOpFunc onOp) {
    ca.updateTestData(data);
    program->deploy(addr, bytes{code});
    program->setBalance(addr, DEFAULT_BALANCE);
    program->updateEnv(ca.decodeAccounts(), ca.decodeBlock());
    program->invoke(addr, CONTRACT_CONSTRUCTOR, ca.encodeConstructor(), ca.isPayable(""), onOp);
  }

  TargetContainerResult TargetExecutive::exec(bytes data, const tuple<unordered_set<uint64_t>, unordered_set<uint64_t>>& validJumpis) {
    /* Save all hit branches to trace_bits */
    Instruction prevInst;
    RecordParam recordParam;
    u256 lastCompValue = 0;
    u64 jumpDest1 = 0;
    u64 jumpDest2 = 0;
    unordered_set<string> uniqExceptions;
    unordered_set<string> tracebits;
    unordered_map<string, u256> predicates;
    vector<bytes> outputs;
    size_t savepoint = program->savepoint();
    OnOpFunc onOp = [&](u64, u64 pc, Instruction inst, bigint, bigint, bigint, VMFace const* _vm, ExtVMFace const* ext) {
      auto vm = dynamic_cast<LegacyVM const*>(_vm);
      /* Oracle analyze data */
      switch (inst) {
        case Instruction::CALL:
        case Instruction::CALLCODE:
        case Instruction::DELEGATECALL:
        case Instruction::STATICCALL: {
          vector<u256>::size_type stackSize = vm->stack().size();
          u256 wei = (inst == Instruction::CALL || inst == Instruction::CALLCODE) ? vm->stack()[stackSize - 3] : 0;
          auto sizeOffset = (inst == Instruction::CALL || inst == Instruction::CALLCODE) ? (stackSize - 4) : (stackSize - 3);
          auto inOff = (uint64_t) vm->stack()[sizeOffset];
          auto inSize = (uint64_t) vm->stack()[sizeOffset - 1];
          auto first = vm->memory().begin();
          OpcodePayload payload;
          payload.caller = ext->myAddress;
          payload.callee = Address((u160)vm->stack()[stackSize - 2]);
          payload.pc = pc;
          payload.gas = vm->stack()[stackSize - 1];
          payload.wei = wei;
          payload.inst = inst;
          payload.data = bytes(first + inOff, first + inOff + inSize);
          oracleFactory->save(OpcodeContext(ext->depth + 1, payload));
          break;
        }
        default: {
          OpcodePayload payload;
          payload.pc = pc;
          payload.inst = inst;
          if (
              inst == Instruction::SUICIDE ||
              inst == Instruction::NUMBER ||
              inst == Instruction::TIMESTAMP ||
              inst == Instruction::INVALID ||
              inst == Instruction::ADD ||
              inst == Instruction::SUB
              ) {
            vector<u256>::size_type stackSize = vm->stack().size();
            if (inst == Instruction::ADD || inst == Instruction::SUB) {
              auto left = vm->stack()[stackSize - 1];
              auto right = vm->stack()[stackSize - 2];
              if (inst == Instruction::ADD) {
                auto total256 = left + right;
                auto total512 = (u512) left + (u512) right;
                payload.isOverflow = total512 != total256;
              }
              if (inst == Instruction::SUB) {
                payload.isUnderflow = left < right;
              }
            }
            oracleFactory->save(OpcodeContext(ext->depth + 1, payload));
          }
          break;
        }
      }
      /* Mutation analyzes data */
      switch (inst) {
        case Instruction::GT:
        case Instruction::SGT:
        case Instruction::LT:
        case Instruction::SLT:
        case Instruction::EQ: {
          vector<u256>::size_type stackSize = vm->stack().size();
          if (stackSize >= 2) {
            u256 left = vm->stack()[stackSize - 1];
            u256 right = vm->stack()[stackSize - 2];
            /* calculate if command inside a function */
            u256 temp = left > right ? left - right : right - left;
            lastCompValue = temp + 1;
          }
          break;
        }
        default: { break; }
      }
      /* Calculate left and right branches for valid jumpis*/
      auto recordable = recordParam.isDeployment && get<0>(validJumpis).count(pc);
      recordable = recordable || !recordParam.isDeployment && get<1>(validJumpis).count(pc);
      if (inst == Instruction::JUMPCI && recordable) {
        jumpDest1 = (u64) vm->stack().back();
        jumpDest2 = pc + 1;
      }
      /* Calculate actual jumpdest and add reverse branch to predicate */
      recordable = recordParam.isDeployment && get<0>(validJumpis).count(recordParam.lastpc);
      recordable = recordable || !recordParam.isDeployment && get<1>(validJumpis).count(recordParam.lastpc);
      if (prevInst == Instruction::JUMPCI && recordable) {
        auto branchId = to_string(recordParam.lastpc) + ":" + to_string(pc);
        tracebits.insert(branchId);
        /* Calculate branch distance */
        u64 jumpDest = pc == jumpDest1 ? jumpDest2 : jumpDest1;
        branchId = to_string(recordParam.lastpc) + ":" + to_string(jumpDest);
        predicates[branchId] = lastCompValue;
      }
      prevInst = inst;
      recordParam.lastpc = pc;
    };
    /* Decode and call functions */
    ca.updateTestData(data);  // 分配data
    vector<bytes> funcs = ca.encodeFunctions();

//    size_t savepoint = program->savepoint();

    /* 原本的是有的，但是现在希望所进行的fuzzing过程，都是基于现在execSpecial过后的结果 */
//    if (hasSpecialState) {
//        // 如果有execSpecial的状态,则恢复到那个状态
//        uniqExceptions = specialResult.uniqExceptions;
//        tracebits = specialResult.tracebits;
//        predicates = specialResult.predicates;
//    } else {
//        // 否则,正常部署合约,并设置余额,更新环境,初始化oracle
//        program->deploy(addr, code);
//        program->setBalance(addr, DEFAULT_BALANCE);
//        program->updateEnv(ca.decodeAccounts(), ca.decodeBlock());
//        oracleFactory->initialize();
//    }

    program->deploy(addr, code);
    program->setBalance(addr, DEFAULT_BALANCE);
    program->updateEnv(ca.decodeAccounts(), ca.decodeBlock());
    oracleFactory->initialize();

    /* Record all JUMPI in constructor */
    recordParam.isDeployment = true;
    auto sender = ca.getSender();
    OpcodePayload payload;
    payload.inst = Instruction::CALL;
    payload.data = ca.encodeConstructor();
    payload.wei = ca.isPayable("") ? program->getBalance(sender) / 2 : 0;
    payload.caller = sender;
    payload.callee = addr;
    oracleFactory->save(OpcodeContext(0, payload));
    auto res = program->invoke(addr, CONTRACT_CONSTRUCTOR, ca.encodeConstructor(), ca.isPayable(""), onOp);
    if (res.excepted != TransactionException::None) {
      auto exceptionId = to_string(recordParam.lastpc);
      uniqExceptions.insert(exceptionId) ;
      /* Save Call Log */
      OpcodePayload payload;
      payload.inst = Instruction::INVALID;
      oracleFactory->save(OpcodeContext(0, payload));
    }
    oracleFactory->finalize();

    flag_show++;
    if (flag_show == 10){
        for (uint32_t funcIdx = 0; funcIdx < funcs.size(); funcIdx ++ ) {
            auto func = funcs[funcIdx];
            auto fd = ca.fds[funcIdx];
            cout << "***PRINT***\nnow, in function `exec`, that is the: " << funcIdx << " time! And the name of calling function is: " << fd.name << endl;

        }
    }

    for (uint32_t funcIdx = 0; funcIdx < funcs.size(); funcIdx ++ ) {
        // 这里的加一，是因为原本的exec中，普通的函数调用和构造函数调用是分开的
        // 但是，我希望在普通函数的fuzzing过程中，使用我们自己指定的函数调用序列进行fuzzing。
        // 那么不可避免的就会用到构造函数。
        // 现在，相当于就是调用了在exec中固定调用一次构造函数、在正式执行函数循环的过程中，又在开头调用了一次构造函数！
        // 也就是说，当前的做法是一共调用了两次构造函数！

        // 额，+1会产生错误，并不+1。。。

      // 第一部分：
      // 改：继续跑给出的序列，而不是原本sFuzz固定的序列 - VFCS
      // 跑给出的固定的序列，用sFuzz的变异策略，测试跑的效果 - 基于VFCS中的参数进行sFuzz本身的变异
      // 用sFuzz的策略来变异参数，然后来测试效果怎么样 —— 先实现

      /* 直接改了 ContractABI中的构造函数，将输入进来的json文件按照order顺序来提取函数相关信息 */
      /*** ✅经测试: 确实有提升，但单一的调用序列始终不足够 ***/


      // 实现一个自动化的接口——LLM给出可以带参数的序列，打包后，能持续的根据提示词生成序列，自动化的调用来持续
      // - 是实验的属性，而不是做出来给别人用的工具 - 给出相应的接口，作为一个额外的东西 pool
        // - 测试效果 - 多长时间内 - 不同LLM - 不同参数的LLM
      // - 覆盖率的统计评估
      // IR-Fuzz只是跑一遍

      // coverage
      // 框架写好之后就一直跑一直跑

        // A, B, C
      /* Update payload */
      auto func = funcs[funcIdx];
      auto fd = ca.fds[funcIdx];
      // 测试一下是不是固定的
      // 直接打印出当前fd的名字就好，可以试试看：
//      cout << "***PRINT***\nnow, in function `exec`, that is the: " << funcIdx << " time! And the name of calling function is: " << fd.name << endl;
      /***  ✅经测试: sFuzz的函数调用序列就是固定的 ***/

      // 得有motivation
      // EVM的指令设计

      // 变异的策略：
      // 选择某个部分，固定某一些字节不让变
      // 在有趣的种子，做一些固定、不让变异
      // 有目的性、选择性的变异策略——可以做针对性优化

      /* Ignore JUMPI until program reaches inside function */
      recordParam.isDeployment = false;
      OpcodePayload payload;
      payload.data = func;
      payload.inst = Instruction::CALL;
      payload.wei = ca.isPayable(fd.name) ? program->getBalance(sender) / 2 : 0;
      payload.caller = sender;
      payload.callee = addr;
      oracleFactory->save(OpcodeContext(0, payload));
      res = program->invoke(addr, CONTRACT_FUNCTION, func, ca.isPayable(fd.name), onOp);
      outputs.push_back(res.output);
      if (res.excepted != TransactionException::None) {
        auto exceptionId = to_string(recordParam.lastpc);
        uniqExceptions.insert(exceptionId);
        /* Save Call Log */
        OpcodePayload payload;
        payload.inst = Instruction::INVALID;
        oracleFactory->save(OpcodeContext(0, payload));
      }
      oracleFactory->finalize();
    }
    /* Reset data before running new contract */
    program->rollback(savepoint);
    string cksum = "";
    for (auto t : tracebits) cksum = cksum + t;
//    cout << "****************************************\nHHHHHHere, will show the 'cksum':" << endl;
//    cout << cksum << endl;
    /*
    ****************************************
    HHHHHHere, will show the 'cksum':
    352:357455:456
    */
    /* 352:357455:456 */  /* 20% */
    /* 851:853701:702701:710455:460352:357860:861455:456 */  /* 80% */

//    cout << "##########################\n Tracebits:" << tracebits << endl;
    /* Tracebits:{ 352:357, 455:456 } */

//    cout << " Predicates:" << predicates << endl;
//    cout << " UniqExceptions:" << uniqExceptions << endl;
    /* UniqExceptions:{ 458 } */

    return TargetContainerResult(tracebits, predicates, uniqExceptions, cksum);
  }





/* new */
TargetContainerResult TargetExecutive::execSpecial(const string& jsonData, const tuple<unordered_set<uint64_t>, unordered_set<uint64_t>>& validJumpis) {
    Instruction prevInst;
    RecordParam recordParam;
    u256 lastCompValue = 0;
    u64 jumpDest1 = 0;
    u64 jumpDest2 = 0;
    unordered_set<string> uniqExceptions;
    unordered_set<string> tracebits;
    unordered_map<string, u256> predicates;
    vector<bytes> outputs;

    size_t savepoint = program->savepoint();

    OnOpFunc onOp = [&](u64, u64 pc, Instruction inst, bigint, bigint, bigint, VMFace const* _vm, ExtVMFace const* ext) {
        auto vm = dynamic_cast<LegacyVM const*>(_vm);
        /* Oracle analyze data */
        switch (inst) {
            case Instruction::CALL:
            case Instruction::CALLCODE:
            case Instruction::DELEGATECALL:
            case Instruction::STATICCALL: {
              vector<u256>::size_type stackSize = vm->stack().size();
              u256 wei = (inst == Instruction::CALL || inst == Instruction::CALLCODE) ? vm->stack()[stackSize - 3] : 0;
              auto sizeOffset = (inst == Instruction::CALL || inst == Instruction::CALLCODE) ? (stackSize - 4) : (stackSize - 3);
              auto inOff = (uint64_t) vm->stack()[sizeOffset];
              auto inSize = (uint64_t) vm->stack()[sizeOffset - 1];
              auto first = vm->memory().begin();
              OpcodePayload payload;
              payload.caller = ext->myAddress;
              payload.callee = Address((u160)vm->stack()[stackSize - 2]);
              payload.pc = pc;
              payload.gas = vm->stack()[stackSize - 1];
              payload.wei = wei;
              payload.inst = inst;
              payload.data = bytes(first + inOff, first + inOff + inSize);
              oracleFactory->save(OpcodeContext(ext->depth + 1, payload));
              break;
            }
            default: {
              OpcodePayload payload;
              payload.pc = pc;
              payload.inst = inst;
              if (
                  inst == Instruction::SUICIDE ||
                  inst == Instruction::NUMBER ||
                  inst == Instruction::TIMESTAMP ||
                  inst == Instruction::INVALID ||
                  inst == Instruction::ADD ||
                  inst == Instruction::SUB
              ) {
                  vector<u256>::size_type stackSize = vm->stack().size();
                  if (inst == Instruction::ADD || inst == Instruction::SUB) {
                      auto left = vm->stack()[stackSize - 1];
                      auto right = vm->stack()[stackSize - 2];
                      if (inst == Instruction::ADD) {
                          auto total256 = left + right;
                          auto total512 = (u512) left + (u512) right;
                          payload.isOverflow = total512 != total256;
                      }
                      if (inst == Instruction::SUB) {
                          payload.isUnderflow = left < right;
                      }
                  }
                  oracleFactory->save(OpcodeContext(ext->depth + 1, payload));
              }
              break;
            }
        }
        /* Mutation analyzes data */
        switch (inst) {
        case Instruction::GT:
        case Instruction::SGT:
        case Instruction::LT:
        case Instruction::SLT:
        case Instruction::EQ: {
            vector<u256>::size_type stackSize = vm->stack().size();
            if (stackSize >= 2){
                u256 left = vm->stack()[stackSize - 1];
                u256 right = vm->stack()[stackSize - 2];
                /* calculate if command inside a function */
                u256 temp = left > right ? left - right : right - left;
                lastCompValue = temp + 1;
            }
            break;
        }
        default: { break; }
        }
        /* Calculate left and right branches for valid jumpis*/
        auto recordable = recordParam.isDeployment && get<0>(validJumpis).count(pc);
        recordable = recordable || !recordParam.isDeployment && get<1>(validJumpis).count(pc);
        if (inst == Instruction::JUMPCI && recordable){
            jumpDest1 = (u64)vm->stack().back();
            jumpDest2 = pc + 1;
        }
        /* Calculate actual jumpdest and add reverse branch to predicate */
        recordable = recordParam.isDeployment && get<0>(validJumpis).count(recordParam.lastpc);
        recordable = recordable || !recordParam.isDeployment && get<1>(validJumpis).count(recordParam.lastpc);
        if (prevInst == Instruction::JUMPCI && recordable){
            auto branchId = to_string(recordParam.lastpc) + ":" + to_string(pc);
            tracebits.insert(branchId);
            /* Calculate branch distance */
            u64 jumpDest = pc == jumpDest1 ? jumpDest2 : jumpDest1;
            branchId = to_string(recordParam.lastpc) + ":" + to_string(jumpDest);
            predicates[branchId] = lastCompValue;
        }
        prevInst = inst;
        recordParam.lastpc = pc;
    };



    // 直接使用specialTestcase生成测试数据
    bytes data = ca.specialTestcase(jsonData);
    ca.updateTestData(data);

    // 部署合约
    program->deploy(addr, code);
    program->setBalance(addr, DEFAULT_BALANCE);
    program->updateEnv(ca.decodeAccounts(), ca.decodeBlock());
    oracleFactory->initialize();

    // 解析 JSON 数据
    pt::ptree root;
    stringstream ss(jsonData);
    pt::read_json(ss, root);

//    cout << "TTTTTTTTTTTTT\nHere, will show the tree about jsonData to `root`:\n" << root << endl;

    bool success = true;

    // 处理构造函数
    for (auto& item : root) {

        string type = item.second.get<string>("type");
        if (type == "constructor") {
            auto params = item.second.get_child("params");

            vector<TypeDef> tds = parseParams(params);

            recordParam.isDeployment = true;
            auto sender = ca.getSender();
            OpcodePayload payload;
            payload.inst = Instruction::CALL;
            payload.data = ca.encodeConstructor();
            payload.wei = ca.isPayable("") ? program->getBalance(sender) / 2 : 0;
            payload.caller = sender;
            payload.callee = addr;
            oracleFactory->save(OpcodeContext(0, payload));

            auto res = program->invoke(addr, CONTRACT_CONSTRUCTOR, ca.encodeConstructor(), ca.isPayable(""), onOp);
            if (res.excepted != TransactionException::None) {
                auto exceptionId = to_string(recordParam.lastpc);
                uniqExceptions.insert(exceptionId) ;
                /* Save Call Log */
                OpcodePayload payload;
                payload.inst = Instruction::INVALID;
                oracleFactory->save(OpcodeContext(0, payload));

                success = false;
            }
            oracleFactory->finalize();
            break;
        }
    }
    cout << "******\nThe Constructor part of `execSpecial` was processed successfully!\n******" << endl;

    bool ifExecFunc = false;
    // 执行函数调用
    for (auto& item : root) {
        string type = item.second.get<string>("type");
        if (type == "function") {  // 执行函数调用
            if (!ifExecFunc) ifExecFunc = true;
            string name = item.second.get<string>("name");
            auto params = item.second.get_child("params");

            vector<TypeDef> tds = parseParams(params);

            bytes functionData = ca.encodeTuple(tds);
            bytes selector = ca.functionSelector(name, tds);
            bytes callData = selector + functionData;

            /* Update payload */
            recordParam.isDeployment = false;
            auto sender = ca.getSender();
            OpcodePayload payload;
            payload.data = callData;
            payload.inst = Instruction::CALL;
            payload.wei = ca.isPayable(name) ? program->getBalance(sender) / 2 : 0;
            payload.caller = sender;
            payload.callee = addr;
            oracleFactory->save(OpcodeContext(0, payload));

            auto res = program->invoke(addr, CONTRACT_FUNCTION, callData, ca.isPayable(name), onOp);
            outputs.push_back(res.output);

            if (res.excepted != TransactionException::None) {
                auto exceptionId = to_string(recordParam.lastpc);
                uniqExceptions.insert(exceptionId);
                /* Save Call Log */
                OpcodePayload payload;
                payload.inst = Instruction::INVALID;
                oracleFactory->save(OpcodeContext(0, payload));

                success = false;
            }
            oracleFactory->finalize();
            cout << "##\nThe `" << name << "` function successfully processed the function part!\n##" << endl;
        }
    }
    if (ifExecFunc)
        cout << "##****** The Function part of `execSpecial` was processed successfully! ******##" << endl;

//    if (success) {
//
//        string cksum = "";
//        for (auto t : tracebits) cksum = cksum + t;
//
//        // 如果成功处理了构造函数和普通函数,则保存当前状态
//        TargetContainerResult specialResult = TargetContainerResult(tracebits, predicates, uniqExceptions, cksum);
//        hasSpecialState = true;
//    }
//
//    /* Uncomment when need to reset the environment */
//    // program->rollback(savepoint);
//
//    // 在保存状态之前,设置余额和更新环境
////    program->setBalance(addr, DEFAULT_BALANCE);
////    program->updateEnv(ca.decodeAccounts(), ca.decodeBlock());
////    oracleFactory->initialize();
//
////    return TargetContainerResult(tracebits, predicates, uniqExceptions, cksum);
//    return specialResult;
//
    /* Uncomment when need to reset the environment */
    // program->rollback(savepoint);

    string cksum = "";
    for (auto t : tracebits) cksum = cksum + t;
    return TargetContainerResult(tracebits, predicates, uniqExceptions, cksum);
}

vector<TypeDef> TargetExecutive::parseParams(const pt::ptree& params) {
    vector<TypeDef> tds;
    for (auto& param : params) {
        string paramName = param.second.get<string>("name");
        string paramType = param.second.get<string>("type");
        auto paramValue = param.second.get_child("value");

        TypeDef td(paramName);
        td.dimensions = td.extractDimension(paramType);
        td.isDynamic = paramType.find("[]") != string::npos;
        td.isDynamicArray = td.isDynamic;
        td.isSubDynamicArray = paramType.find("[][]") != string::npos;
        td.padLeft = td.toRealname(paramType).find("uint") != string::npos;

        if (td.dimensions.size() == 0) {
            // 单个值
            td.addValue(unpackBytes(paramValue.data()));
        } else if (td.dimensions.size() == 1) {
            // 一维数组
            vector<bytes> vs;
            for (auto& v : paramValue) {
                vs.push_back(unpackBytes(v.second.data()));
            }
            td.addValue(vs);
        } else if (td.dimensions.size() == 2) {
            // 二维数组
            vector<vector<bytes>> vss;
            for (auto& vs : paramValue) {
                vector<bytes> sub;
                for (auto& v : vs.second) {
                    sub.push_back(unpackBytes(v.second.data()));
                }
                vss.push_back(sub);
            }
            td.addValue(vss);
        }

        tds.push_back(td);
    }
    return tds;
}

bytes TargetExecutive::unpackBytes(const string& data) {
    if (data.substr(0, 2) == "0x") {
        // 如果数据以 "0x" 开头,则假设它是一个十六进制字符串
        return fromHex(data.substr(2));
    } else {
        // 否则,假设它是一个普通的字符串,直接将其转换为 bytes
        return bytes(data.begin(), data.end());
    }
}

}
