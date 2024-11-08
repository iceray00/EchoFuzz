#include <regex>
#include "ContractABI.h"
#include <boost/optional.hpp>

using namespace std;
namespace pt = boost::property_tree;

namespace fuzzer {
  FuncDef::FuncDef(string name, vector<TypeDef> tds, bool payable) {
    this->name = name;
    this->tds = tds;
    this->payable = payable;
  }

  FakeBlock ContractABI::decodeBlock() {
    if (!block.size()) throw "Block is empty";
    auto numberInBytes = bytes(block.begin(), block.begin() + 8);
    auto timestampInBytes = bytes(block.begin() + 8, block.begin() + 16);
    auto number = u64("0x" + toHex(numberInBytes));
    auto timestamp = u64("0x" + toHex(timestampInBytes));
    return make_tuple(block, (int64_t)number, (int64_t)timestamp);
  }

  Address ContractABI::getSender() {
    auto accounts = decodeAccounts();
    for (auto account : accounts) {
      if (get<3>(account)) return get<1>(account);
    }
  }

  Accounts ContractABI::decodeAccounts() {
    unordered_set<string> accountSet;
    Accounts ret;
    auto isSender = true;
    for (auto account : accounts) {
      bytes balanceInBytes(account.begin(), account.begin() + 12);
      bytes addressInBytes(account.begin() + 12, account.end());
      u256 balance = u256("0x" + toHex(balanceInBytes));
      u160 address = u160("0x" + toHex(addressInBytes));
      auto pair = accountSet.insert(toHex(addressInBytes));
      if (pair.second) {
        ret.push_back(make_tuple(account, address, balance, isSender));
        isSender = false;
      }
    }
    return ret;
  }
  
  uint64_t ContractABI::totalFuncs() {
    return count_if(fds.begin(), fds.end(), [](FuncDef fd) {
      return fd.name != "";
    });
  }
  
  string ContractABI::toStandardJson() {
    stringstream os;
    pt::ptree funcs;
    pt::ptree root;
    for (auto fd : this->fds) {
      pt::ptree func;
      pt::ptree inputs;
      func.put("name", fd.name);
      for (auto td : fd.tds) {
        pt::ptree input;
        input.put("type", td.name);
        switch (td.dimensions.size()) {
          case 0: {
            input.put("value", "0x" + toHex(td.dt.value));
            break;
          }
          case 1: {
            pt::ptree values;
            for (auto dt : td.dts) {
              pt::ptree value;
              value.put_value("0x" + toHex(dt.value));
              values.push_back(make_pair("", value));
            }
            input.add_child("value", values);
            break;
          }
          case 2: {
            pt::ptree valuess;
            for (auto dts : td.dtss) {
              pt::ptree values;
              for (auto dt : dts) {
                pt::ptree value;
                value.put_value("0x" + toHex(dt.value));
                values.push_back(make_pair("", value));
              }
              valuess.push_back(make_pair("", values));
            }
            input.add_child("value", valuess);
            break;
          }
        }
        inputs.push_back(make_pair("", input));
      }
      func.add_child("inputs", inputs);
      funcs.push_back(make_pair("", func));
    }
    root.add_child("functions", funcs);
    /* Accounts */
    unordered_set<string> accountSet; // to check exists
    pt::ptree accs;
    auto accountInTuples = decodeAccounts();
    for (auto account : accountInTuples) {
      auto accountInBytes = get<0>(account);
      auto balance = get<2>(account);
      auto address = bytes(accountInBytes.begin() + 12, accountInBytes.end());
      pt::ptree acc;
      acc.put("address", "0x" + toHex(address));
      acc.put("balance", balance);
      accs.push_back(make_pair("", acc));
    }
    root.add_child("accounts", accs);
    pt::write_json(os, root);
    return os.str();
  }
  /*
   * Validate generated data before sending it to vm
   * msg.sender address can not be 0 (32 - 64)
   */
  // postprocessTestData 函数对 data 进行预处理，确保发送者地址和余额等字段符合 EVM 的要求。
  bytes ContractABI::postprocessTestData(bytes data) {
    auto sender = u256("0x" + toHex(bytes(data.begin() + 44, data.begin() + 64)));
    auto balance = u256("0x" + toHex(bytes(data.begin() + 32, data.begin() + 44)));
    if (!balance) data[32] = 0xff;
    if (!sender) data[63] = 0xf0;
    return data;
  }

  /*
   * ContractABI::updateTestData 函数根据 data 中的内容更新智能合约的测试数据。
   * 这个函数解析 data 中的每个参数，并将其转换为智能合约可以理解的格式。
   * 例如，如果 data 中包含地址或余额信息，这个函数会将其提取出来并更新到相应的账户状态中。
   */
  void ContractABI::updateTestData(bytes data) {

    // 从传入进来的 data 数据的前32位中，每一位都是对应着可能需要的动态数组的长度信息。
    /* Detect dynamic len by consulting first 32 bytes */
    int lenOffset = 0;
    auto consultRealLen = [&]() {
      int len = data[lenOffset];
      lenOffset = (lenOffset + 1) % 32;
      return len;
    };

    // 获取当前长度以32为倍数的最小值
    /* Container of dynamic len */
    auto consultContainerLen = [](int realLen) {
      if (!(realLen % 32)) return realLen;
      return (realLen / 32 + 1) * 32;
    };

    // padding 对动态类型的参数，用0填充不足32为倍数的部分
    /* Pad to enough data before decoding */
    int offset = 96;
    auto padLen = [&](int singleLen) {
      int fitLen = offset + singleLen;
      while ((int)data.size() < fitLen) data.push_back(0);
    };


    block.clear();
    accounts.clear();
    auto senderInBytes = bytes(data.begin() + 32, data.begin() + 64);
    block = bytes(data.begin() + 64, data.begin() + 96);
    accounts.push_back(senderInBytes);

    // 对每个函数
    for (auto &fd : this->fds) {
      // 对当前函数中的每个参数
      for (auto &td : fd.tds) {
        switch (td.dimensions.size()) {
          case 0: {
            // 当前的长度：如果是动态的数组，就去传入进来的data的前32个字节中找对应的长度
            // 前32个字节，每一个字节的值都对应着这里的动态长度的长度值。
            int realLen = td.isDynamic ? consultRealLen() : 32;

            // 如果当前的参数是动态的，且已经获取完具体的数值了。
            // 现在所需要的容器长度就是得以32为倍数的
            int containerLen = consultContainerLen(realLen);

            /* Pad to enough bytes to read */
            // 根据完整划分的容器长度，对需要填充的部分就用 0 进行填补
            padLen(containerLen);

            /* Read from offset ... offset + realLen */
            bytes d(data.begin() + offset, data.begin() + offset + realLen);
            // data.begin()+offset 就是跳过前面的内容，直接到参数的位置
            // 再加上当前的真实长度，也就是真实数据再data中的值

            /* If address, extract account */
            // 如果当前的参数类型是地址，那么就把当前的真实地址压入accounts中
            if (boost::starts_with(td.name, "address")) {
              accounts.push_back(d);
            }
            // 给类型数据添加是必不可少的
            td.addValue(d);

            /* Ignore (containerLen - realLen) bytes */
            // 中间填充了的部分就不管了
            offset += containerLen;
            // offset加上现在单独参数扩充后的容器长度
            break;
          }
          case 1: {
            vector<bytes> ds;
            int numElem = td.dimensions[0] ? td.dimensions[0] : consultRealLen();
            for (int i = 0; i < numElem; i += 1) {
              int realLen = td.isDynamic ? consultRealLen() : 32;
              int containerLen = consultContainerLen(realLen);
              padLen(containerLen);
              bytes d(data.begin() + offset, data.begin() + offset + realLen);
              ds.push_back(d);
              offset += containerLen;
            }
            /* If address, extract account */
            if (boost::starts_with(td.name, "address")) {
              accounts.insert(accounts.end(), ds.begin(), ds.end());
            }
            td.addValue(ds);
            break;
          }
          case 2: {
            vector<vector<bytes>> dss;
            int numElem = td.dimensions[0] ? td.dimensions[0] : consultRealLen();
            int numSubElem = td.dimensions[1] ? td.dimensions[1] : consultRealLen();
            for (int i = 0; i < numElem; i += 1) {
              vector<bytes> ds;
              for (int j = 0; j < numSubElem; j += 1) {
                int realLen = td.isDynamic ? consultRealLen() : 32;
                int containerLen = consultContainerLen(realLen);
                padLen(containerLen);
                bytes d(data.begin() + offset, data.begin() + offset + realLen);
                ds.push_back(d);
                offset += containerLen;
              }
              dss.push_back(ds);
              /* If address, extract account */
              if (boost::starts_with(td.name, "address")) {
                accounts.insert(accounts.end(), ds.begin(), ds.end());
              }
            }
            td.addValue(dss);
            break;
          }
        }
      }
    }
  }

  bytes ContractABI::randomTestcase() {
    /*
     * Random value for ABI
     * | --- dynamic len (32 bytes) -- | sender | blockNumber(8) + timestamp(8) | content |
     */
    bytes ret(32, 5);
    int lenOffset = 0;
    auto consultRealLen = [&]() {
      int len = ret[lenOffset];
      lenOffset = (lenOffset + 1) % 32;
      return len;
    };
    auto consultContainerLen = [](int realLen) {
      if (!(realLen % 32)) return realLen;
      return (realLen / 32 + 1) * 32;
    };
    /* sender env */
    bytes sender(32, 0);
    bytes block(32, 0);
    ret.insert(ret.end(), sender.begin(), sender.end());
    ret.insert(ret.end(), block.begin(), block.end());
    for (auto fd : this->fds) {
      for (auto td : fd.tds) {
        switch(td.dimensions.size()) {
          case 0: {
            int realLen = td.isDynamic ? consultRealLen() : 32;
            int containerLen = consultContainerLen(realLen);
            bytes data(containerLen, 0);
            ret.insert(ret.end(), data.begin(), data.end());
            break;
          }
          case 1: {
            int numElem = td.dimensions[0] ? td.dimensions[0] : consultRealLen();
            for (int i = 0; i < numElem; i += 1) {
              int realLen = td.isDynamic ? consultRealLen() : 32;
              int containerLen = consultContainerLen(realLen);
              bytes data = bytes(containerLen, 0);
              ret.insert(ret.end(), data.begin(), data.end());
            }
            break;
          }
          case 2: {
            int numElem = td.dimensions[0] ? td.dimensions[0] : consultRealLen();
            int numSubElem = td.dimensions[1] ? td.dimensions[1] : consultRealLen();
            for (int i = 0; i < numElem; i += 1) {
              for (int j = 0; j < numSubElem; j += 1) {
                int realLen = td.isDynamic ? consultRealLen() : 32;
                int containerLen = consultContainerLen(realLen);
                bytes data = bytes(containerLen, 0);
                ret.insert(ret.end(), data.begin(), data.end());
              }
            }
            break;
          }
        }
      }
    }
    return ret;
  }

    ContractABI::ContractABI(string abiJson) {
        transactionLength = 1;  /* new */
        stringstream ss;
        ss << abiJson;
        pt::ptree root;
        pt::read_json(ss, root);

        // std::cout << "before : " << this->fds.size() << std::endl;

        vector<int> allOrders;
    // 带有order字段的总长度
    //    /* First iterate over all orders to determine the size of the fds */
    //    for (auto node : root) {
    //      if (node.second.get_child_optional("order")) {
    //        auto orderType = node.second.get<string>("order_type");
    //        if (orderType == "int") {
    //          int order = node.second.get<int>("order");
    //          allOrders.push_back(order);
    //        } else if (orderType == "list") {
    //          auto orderNode = node.second.get_child("order");
    //          for (auto &orderVal : orderNode) {
    //            int order = orderVal.second.get_value<int>();
    //            allOrders.push_back(order);
    //          }
    //        }
    //      }
    //    }
        for (auto node : root) {
            string type = node.second.get<string>("type");
            if (type == "constructor")
                continue;
            if (node.second.get_child_optional("order") && !node.second.get_child("order").empty()){
                auto orderNode = node.second.get_child("order");
                for (auto &orderVal : orderNode) {
                    int order = orderVal.second.get_value<int>();
                    allOrders.push_back(order);
                }
            }
        }

        /* Determine the size of the fds */
        //    int maxOrder = *max_element(allOrders.begin(), allOrders.end());
        int lengthOrder = allOrders.size();
        // order字段的总长度

        this->fds.resize(lengthOrder + 1);  // 可以不+1


        vector<int> notOrders;
        /* 如果符合加入条件、且不带有order字段，那么就统一放在前面。将所有含有order字段的函数调用都放在后面 */
        // 因此，需要先计算出：有多少个是不带有order字段、且符合加入条件的函数。
        for (auto node: root) {
            string constant = "false";
            // constant
            if (node.second.get_child_optional("constant")) {
                constant = node.second.get<string>("constant");
            }
            string type = node.second.get<string>("type");
            if (type == "fallback" && !(node.second.get_child_optional("order"))){
                notOrders.push_back(1);
            }
            if ((type == "constructor" || type == "function") && constant == "false" && !(node.second.get_child_optional("order"))){
                notOrders.push_back(2);
            }
        }
        int lengthNotOrder = notOrders.size();

        this->fds.resize(2 * lengthOrder + lengthNotOrder + 1);  // +1: add the number of constructor from A2
        /* | ----- lengthOrder ----- | ---- lengthNotOrder ---- | ----- lengthOrder + 1 ----- | */


        // A1: Add a function with an order field to the fds
        for (auto node : root) {
            vector<TypeDef> tds;
            string type = node.second.get<string>("type");
            string constant = "false";
            bool payable = false;
            // constant
            if (node.second.get_child_optional("constant")) {
                constant = node.second.get<string>("constant");
            }
            // fallback
            if (type == "fallback" && node.second.get_child_optional("order")) {
                if (node.second.get_child_optional("payable")) {
                    payable = node.second.get<bool>("payable");
                }
    //        /* order is list */
    //        auto orderNode = node.second.get_child("order");
    //        if (is_same<decltype(orderNode), int>::value) {  // That's pure numbers
    //          // If the "order" field is a single integer
    //          int order = orderNode;  // 如果是纯数字，那么当前的order就是orderNode，都是数值
    //          this->fds[order] = FuncDef("fallback", tds, payable);
    //          if (order > realSize) realSize = order;
    //          /* test */
    //          std::cout << "fallback : " << order << std::endl;
    //        } else {
    //          // If the "order" field is an array of integers
    //          for (auto orderVal : orderNode) {
    //            int order = orderVal;  // 因为orderNode就是一个数组，因此不用提取值，本身就是对应的值
    //            this->fds[order] = FuncDef("fallback", tds, payable);
    //            if (order > realSize) realSize = order;
    //            /* test */
    //            std::cout << "fallback : " << order << std::endl;
    //          }
    //        }

    //        auto orderType = node.second.get<string>("order_type");
    //        if (orderType == "int") {  // int
    //          int order = node.second.get<int>("order");
    //          std::cout << "entry `order` as 'int' successfully! In **fallback**" << std::endl;
    //          std::cout << "fallback: " << order << std::endl;
    //          this->fds[order] = FuncDef("fallback", tds, payable);
    ////          if (order > realSize) realSize = order;
    //        } else if (orderType == "list") {  // list
    //          auto orderNode = node.second.get_child("order");
    //          for (auto &orderVal : orderNode) {
    //            std::cout << "into the 'catch' of **\"fallback\"**, that means: `order` is a list!" << std::endl;
    //            int order = orderVal.second.get_value<int>();
    //            std::cout << "fallback: " << order << std::endl;
    //            this->fds[order] = FuncDef("fallback", tds, payable);
    ////            if (order > realSize) realSize = order;
    //          }
    //        }
                auto orderNode = node.second.get_child("order");
                if (orderNode.empty())
                    continue;
                for (auto &orderVal : orderNode) {
                    int order = orderVal.second.get_value<int>();
                    this->fds[order] = FuncDef("fallback", tds, payable);
                    /* test */
                    std::cout << "### A1 Order! " << "fallback" << " : " << order << std::endl << "*****\n";
                }

    //        try {
    //          int order = node.second.get<int>("order");
    //          std::cout << "entry `order` as 'int' successfully! In **fallback**" << std::endl;
    //          std::cout << "fallback: " << order << std::endl;
    //          this->fds[order] = FuncDef("fallback", tds, payable);
    //          if (order > realSize) realSize = order;
    //        } catch (const boost::property_tree::ptree_bad_data &e) {
    //          for (auto &orderVal : orderNode) {
    //            std::cout << "into the 'catch' of **\"fallback\"**, that means: `order` is a list!" << std::endl;
    //            int order = orderVal.second.get_value<int>();
    //            std::cout << "fallback: " << order << std::endl;
    //            this->fds[order] = FuncDef("fallback", tds, payable);
    //            if (order > realSize) realSize = order;
    //          }
    //        }
    //        /* order is int */
    //        int order = node.second.get<int>("order");
    //        this->fds.push_back(FuncDef("fallback", tds, payable));
    //        this->fds[order] = FuncDef("fallback", tds, payable);
    //        if (order > realSize) realSize = order;
            }


            // function -- At that time, A1 doesn't need a constructor
            //      if ((type == "constructor" || type == "function") && constant == "false" && node.second.get_child_optional("order")) {
            if (type == "function" && constant == "false" && node.second.get_child_optional("order")) {
                string name = type == "constructor" ? "" : node.second.get<string>("name");
                if (node.second.get_child_optional("payable")) {
                    payable = node.second.get<bool>("payable");
                }

                auto inputNodes = node.second.get_child("inputs");
                for (auto inputNode : inputNodes) {
                    string type = inputNode.second.get<string>("type");
                    tds.push_back(TypeDef(type));
                }

    //        auto orderType = node.second.get<string>("order_type");
    //        if (orderType == "int") {
    //          int order = node.second.get<int>("order");
    //          std::cout << "entry `order` as 'int' successfully! In **functions**" << std::endl;
    //          std::cout << name << " : " << order << std::endl;
    //          this->fds[order] = FuncDef(name, tds, payable);
    ////          if (order > realSize) realSize = order;
    //          /* test */
    //          std::cout << name << " : " << order << std::endl << "*****\n";
    //        } else if (orderType == "list") {
    //          auto orderNode = node.second.get_child("order");
    //          for (auto &orderVal : orderNode) {
    //            std::cout << "into the 'catch' of **\"functions\"**, that means: `order` is a list!" << std::endl;
    //            int order = orderVal.second.get_value<int>();
    //            std::cout << name << " : " << order << std::endl;
    //            this->fds[order] = FuncDef(name, tds, payable);
    ////            if (order > realSize) realSize = order;
    //            /* test */
    //            std::cout << name << " : " << order << std::endl << "*****\n";
    //          }
    //        }

                auto orderNode = node.second.get_child("order");
                if (orderNode.empty())
                    continue ;
                for (auto &orderVal: orderNode) {
                    int order = orderVal.second.get_value<int>();
                    this->fds[order] = FuncDef(name, tds, payable);
                    /* test */
                    std::cout << "### A1 Order! Function: " << name << " : " << order << std::endl << "*****\n";
                }


    //        /* Try to use exception catching for the int and list cases */
    //        auto orderNode = node.second.get_child("order");
    //        try {
    //          int order = node.second.get<int>("order");
    //          std::cout << "entry `order` as 'int' successfully! In **functions**" << std::endl;
    //          std::cout << "fallback: " << order << std::endl;
    //          this->fds[order] = FuncDef("fallback", tds, payable);
    //          if (order > realSize) realSize = order;
    //          /* test */
    //          std::cout << name << " : " << order << std::endl;
    //        } catch (const boost::property_tree::ptree_bad_data &e) {
    //          for (auto &orderVal : orderNode) {
    //            std::cout << "into the 'catch' of **\"functions\"**, that means: `order` is a list!" << std::endl;
    //            int order = orderVal.second.get_value<int>();
    //            std::cout << "fallback: " << order << std::endl;
    //            this->fds[order] = FuncDef("fallback", tds, payable);
    //            if (order > realSize) realSize = order;
    //            /* test */
    //            std::cout << name << " : " << order << std::endl;
    //          }
    //        }

    //        /* order is list */
    //        auto inputNodes = node.second.get_child("inputs");
    //        for (auto inputNode : inputNodes) {
    //          string type = inputNode.second.get<string>("type");
    //          tds.push_back(TypeDef(type));
    //        }
    //        auto orderNode = node.second.get_child("order");
    //        if (is_same<decltype(orderNode), int>::value) {  // 也就是纯数字
    //          // If the "order" field is a single integer
    //          int order = orderNode;  // 如果是纯数字，那么当前的order就是orderNode，都是数值
    //          this->fds[order] = FuncDef(name, tds, payable);
    //          if (order > realSize) realSize = order;
    //          /* test */
    //          std::cout << name << " : " << order << std::endl;
    //        } else {
    //          // If the "order" field is an array of integers
    //          for (auto orderVal : orderNode) {
    //            int order = orderVal;
    //            this->fds[order] = FuncDef(name, tds, payable);
    //            if (order > realSize) realSize = order;
    //            /* test */
    //            std::cout << name << " : " << order << std::endl;
    //          }
    //        }
    //        /* order is int */
    //        int order = node.second.get<int>("order");
    //        this->fds.push_back(FuncDef(name, tds, payable));
    //        this->fds[order] = FuncDef(name, tds, payable);
    //        if (order > realSize) realSize = order;
            }
        };

        /* test A1 */
        //std::cout << "after : " << this->fds.size() << std::endl;
        std::cout << "A1: FDS size: " << lengthOrder << std::endl;
        for (size_t i = 0; i < lengthOrder; ++i) {
            std::cout << "A1: FDS[" << i << "]: " << this->fds[i].name << std::endl;
        }

        /* ----------------------------------------------------------------- */

        int indexB = 0;
        // B: Add a function to the fds that meets the inclusion criteria, but doesn't have an order field
        for (auto node : root) {
            if (indexB >= lengthNotOrder) break;
            vector<TypeDef> tds;
            string type = node.second.get<string>("type");
            string constant = "false";
            bool payable = false;
            // constant
            if (node.second.get_child_optional("constant")) {
                constant = node.second.get<string>("constant");
            }
            // fallback
            if (type == "fallback" && !(node.second.get_child_optional("order"))) {
                if (node.second.get_child_optional("payable")) {  // 获取payable状态
                    payable = node.second.get<bool>("payable");
                }
                this->fds[lengthOrder + indexB++] = FuncDef("fallback", tds, payable);
                /* test */
                std::cout << "*** Here, B NotOrder! Fallback" << " : " << lengthOrder + indexB -1 << std::endl;
            }
            // function
            if ((type == "constructor" || type == "function") && constant == "false" && !(node.second.get_child_optional("order"))) {
                string name = type == "constructor" ? "" : node.second.get<string>("name");
                if (node.second.get_child_optional("payable")) {
                    payable = node.second.get<bool>("payable");
                }
                auto inputNodes = node.second.get_child("inputs");
                for (auto inputNode : inputNodes) {
                    string type = inputNode.second.get<string>("type");
                    tds.push_back(TypeDef(type));
                }
                this->fds[lengthOrder + indexB++] = FuncDef(name, tds, payable);
                /* test */
                std::cout << "*** Here, B NotOrder! Function" << name << " : " << lengthOrder + indexB -1 << std::endl;
            }
        };
        /* test B */
        std::cout << "B: FDS size:" << lengthNotOrder << std::endl;
        for (size_t i = lengthOrder; i < lengthOrder + lengthNotOrder; ++i){
            std::cout << "B: FDS[" << i << "]:" << this->fds[i].name << std::endl;
        }

        /* ----------------------------------------------------------------- */

        /* A2: Append sequences with an order field that meet the join criteria to the end */
        /*    lengthOrder + lengthNotOrder  -->  lengthOrder + lengthNotOrder + lengthOrder + 1 */
        /* A2: length = lengthOrder + 1. order range in: [0, lengthOrder] */
        for (auto node : root) {
            vector<TypeDef> tds;
            string type = node.second.get<string>("type");
            string constant = "false";
            bool payable = false;
            // constant
            if (node.second.get_child_optional("constant")) {
                constant = node.second.get<string>("constant");
            }
            // fallback
            if (type == "fallback" && node.second.get_child_optional("order")) {
                if (node.second.get_child_optional("payable")) {
                    payable = node.second.get<bool>("payable");
                }
                auto orderNode = node.second.get_child("order");
                if (orderNode.empty())
                    continue;
                for (auto &orderVal : orderNode) {
                    int order = orderVal.second.get_value<int>();
                    this->fds[lengthOrder + lengthNotOrder + order] = FuncDef("fallback", tds, payable);
                    /* test Order */
                    cout << "### A2 Order! Fallback : " << lengthOrder+lengthNotOrder+order << endl;
                }
            }
            // function
            if ((type == "constructor" || type == "function") && constant == "false" && node.second.get_child_optional("order")) {
                string name = type == "constructor" ? "" : node.second.get<string>("name");
                if (node.second.get_child_optional("payable")) {
                    payable = node.second.get<bool>("payable");
                }
                auto inputNodes = node.second.get_child("inputs");
                for (auto inputNode : inputNodes) {
                    string type = inputNode.second.get<string>("type");
                    tds.push_back(TypeDef(type));
                }
                auto orderNode = node.second.get_child("order");
                if (orderNode.empty())
                    continue;
                for (auto &orderVal : orderNode) {
                    int order = orderVal.second.get_value<int>();
                    this->fds[lengthOrder + lengthNotOrder + order] = FuncDef(name, tds, payable);
                    /* test Order */
                    std::cout << "### A2 Order! Function:" << name << " : " << lengthOrder+lengthNotOrder+order << endl;
                }
            }
        };
        /* test A2 */
        std::cout << "A2: FDS size: " << lengthOrder << std::endl;
        for (size_t i = lengthOrder+lengthNotOrder; i < 2*lengthOrder+lengthNotOrder + 1; ++i) {
            std::cout << "A2: FDS[" << i << "]:" << this->fds[i].name << std::endl;
        }

        cout << "********************************************************" << endl;
        cout << "--------------------------------------------------------" << endl;

        /* total sequence */
        cout << "Testing Total Sequence!" << endl;
        for (size_t i = 0; i < 2*lengthOrder + lengthNotOrder +1; ++i) {
            std::cout << "Total FDS[" << i << "]:" << this->fds[i].name << std::endl;
        }

        /* to fix the position of Constructor */
        size_t flagHaveConst = 0;
        for (size_t i = 0; i < 2*lengthOrder + lengthNotOrder -1; ++i){
            if (this->fds[i].name == "")
                flagHaveConst = 1;
        }

        if (flagHaveConst) {
            // Find the location of the constructor
            int constructorIndex = -1;
            FuncDef constructorFuncDef("", vector<TypeDef>(), false);
            for (size_t i = 0; i < this->fds.size(); ++i) {
                if (this->fds[i].name == "") {
                    constructorIndex = i;
                    constructorFuncDef = this->fds[i];
                    break;
                }
            }

            // If constructor is found
            if (constructorIndex != -1) {
                // Moves all functions after the constructor one bit forward
                for (size_t i = constructorIndex; i < this->fds.size() - 2; ++i) {  //
                    this->fds[i] = this->fds[i + 1];
                }
                // Place the constructor last bit
                this->fds[this->fds.size() - 1] = constructorFuncDef;
            }
            /* View total sequence */
            cout << "After Moving the CONSTRUCTOR, the FDS!" << endl;
            for (size_t i = 0; i < 2 * lengthOrder + lengthNotOrder + 1; ++i) {
                std::cout << "FDS[" << i << "]:" << this->fds[i].name << std::endl;
            }
        } else {
            cout << "Constructor's position is good!" << endl;
        }
    }



ContractABI::ContractABI(string abiJson, string orderS, bool isMain) {
    stringstream ss;
    ss << abiJson;
    pt::ptree root;
    pt::read_json(ss, root);

    vector<int> allOrders;
    for (auto node : root) {
        string type = node.second.get<string>("type");
        if (type == "constructor")
            continue;
        if (node.second.get_child_optional("order") && !node.second.get_child("order").empty()){
            auto orderNode = node.second.get_child("order");
            for (auto &orderVal : orderNode) {
                int order = orderVal.second.get_value<int>();
                allOrders.push_back(order);
            }
        }
    }
    /* Determine the size of the fds */
    //    int lengthOrder = *max_element(allOrders.begin(), allOrders.end());
    // The total length of the order field, excluding constructors
    int lengthOrder = allOrders.size();


    this->fds.resize(lengthOrder + 1);  // 可以不+1

    vector<int> notOrders;
    /* 如果符合加入条件、且不带有order字段，那么就统一放在前面。将所有含有order字段的函数调用都放在后面 */
    // 因此，需要先计算出：有多少个是不带有order字段、且符合加入条件的函数。
    for (auto node: root) {
        string constant = "false";
        // constant
        if (node.second.get_child_optional("constant")) {
            constant = node.second.get<string>("constant");
        }
        string type = node.second.get<string>("type");
        if (type == "fallback" && !(node.second.get_child_optional("order"))){
            notOrders.push_back(1);
        }
        if ((type == "constructor" || type == "function") && constant == "false" && !(node.second.get_child_optional("order"))){
            notOrders.push_back(2);
        }
    }
    int lengthNotOrder = notOrders.size();

    if (orderS == "ABA" && isMain) {
        std::cout << "################################## ABA ##################################" << std:: endl;
        this->fds.resize(lengthOrder + lengthNotOrder + lengthOrder + 1);  // +1: add the number of constructor from A2
        /* | ----- lengthOrder ----- | ---- lengthNotOrder ---- | ----- lengthOrder + 1 ----- | */

        // A1: Add a function with an order field to the fds
        for (auto node : root) {
            vector<TypeDef> tds;
            string type = node.second.get<string>("type");
            string constant = "false";
            bool payable = false;
            // constant
            if (node.second.get_child_optional("constant")) {
                constant = node.second.get<string>("constant");
            }
            // fallback
            if (type == "fallback" && node.second.get_child_optional("order")) {
                if (node.second.get_child_optional("payable")) {
                    payable = node.second.get<bool>("payable");
                }
                auto orderNode = node.second.get_child("order");
                if (orderNode.empty())
                    continue;
                for (auto &orderVal : orderNode) {
                    int order = orderVal.second.get_value<int>();
                    this->fds[order] = FuncDef("fallback", tds, payable);
                    /* test */
                    std::cout << "### A1 Order! " << "fallback" << " : " << order << std::endl << "*****\n";
                }
            }
            // function -- At that time, A1 doesn't need a constructor
            //      if ((type == "constructor" || type == "function") && constant == "false" && node.second.get_child_optional("order")) {
            if (type == "function" && constant == "false" && node.second.get_child_optional("order")) {
                string name = type == "constructor" ? "" : node.second.get<string>("name");
                if (node.second.get_child_optional("payable")) {
                    payable = node.second.get<bool>("payable");
                }

                auto inputNodes = node.second.get_child("inputs");
                for (auto inputNode : inputNodes) {
                    string type = inputNode.second.get<string>("type");
                    tds.push_back(TypeDef(type));
                }

                auto orderNode = node.second.get_child("order");
                if (orderNode.empty())
                    continue ;
                for (auto &orderVal: orderNode) {
                    int order = orderVal.second.get_value<int>();
                    this->fds[order] = FuncDef(name, tds, payable);
                    /* test */
                    std::cout << "### A1 Order! Function: " << name << " : " << order << std::endl << "*****\n";
                }
            }
        };
        /* test A1 */
        //std::cout << "after : " << this->fds.size() << std::endl;
        std::cout << "A1: FDS size: " << lengthOrder << std::endl;
        for (size_t i = 0; i < lengthOrder; ++i) {
            std::cout << "A1: FDS[" << i << "]: " << this->fds[i].name << std::endl;
        }

        /* ----------------------------------------------------------------- */

        int indexB = 0;
        // B: Add a function to the fds that meets the inclusion criteria, but doesn't have an order field
        for (auto node : root) {
            if (indexB >= lengthNotOrder) break;
            vector<TypeDef> tds;
            string type = node.second.get<string>("type");
            string constant = "false";
            bool payable = false;
            // constant
            if (node.second.get_child_optional("constant")) {
                constant = node.second.get<string>("constant");
            }
            // fallback
            if (type == "fallback" && !(node.second.get_child_optional("order"))) {
                if (node.second.get_child_optional("payable")) {  // 获取payable状态
                    payable = node.second.get<bool>("payable");
                }
                this->fds[lengthOrder + indexB++] = FuncDef("fallback", tds, payable);
                /* test */
                std::cout << "*** Here, B NotOrder! Fallback" << " : " << lengthOrder + indexB -1 << std::endl;
            }
            // function
            if ((type == "constructor" || type == "function") && constant == "false" && !(node.second.get_child_optional("order"))) {
                string name = type == "constructor" ? "" : node.second.get<string>("name");
                if (node.second.get_child_optional("payable")) {
                    payable = node.second.get<bool>("payable");
                }
                auto inputNodes = node.second.get_child("inputs");
                for (auto inputNode : inputNodes) {
                    string type = inputNode.second.get<string>("type");
                    tds.push_back(TypeDef(type));
                }
                this->fds[lengthOrder + indexB++] = FuncDef(name, tds, payable);
                /* test */
                std::cout << "*** Here, B NotOrder! Function" << name << " : " << lengthOrder + indexB -1 << std::endl;
            }
        };
        /* test B */
        std::cout << "B: FDS size:" << lengthNotOrder << std::endl;
        for (size_t i = lengthOrder; i < lengthOrder + lengthNotOrder; ++i){
            std::cout << "B: FDS[" << i << "]:" << this->fds[i].name << std::endl;
        }

        /* ----------------------------------------------------------------- */

        /* A2: Append sequences with an order field that meet the join criteria to the end */
        /*    lengthOrder + lengthNotOrder  -->  lengthOrder + lengthNotOrder + lengthOrder + 1 */
        /* A2: length = lengthOrder + 1. order range in: [0, lengthOrder] */
        for (auto node : root) {
            vector<TypeDef> tds;
            string type = node.second.get<string>("type");
            string constant = "false";
            bool payable = false;
            // constant
            if (node.second.get_child_optional("constant")) {
                constant = node.second.get<string>("constant");
            }
            // fallback
            if (type == "fallback" && node.second.get_child_optional("order")) {
                if (node.second.get_child_optional("payable")) {
                    payable = node.second.get<bool>("payable");
                }
                auto orderNode = node.second.get_child("order");
                if (orderNode.empty())
                    continue;
                for (auto &orderVal : orderNode) {
                    int order = orderVal.second.get_value<int>();
                    this->fds[lengthOrder + lengthNotOrder + order] = FuncDef("fallback", tds, payable);
                    /* test Order */
                    cout << "### A2 Order! Fallback : " << lengthOrder+lengthNotOrder+order << endl;
                }
            }
            // function
            if ((type == "constructor" || type == "function") && constant == "false" && node.second.get_child_optional("order")) {
                string name = type == "constructor" ? "" : node.second.get<string>("name");
                if (node.second.get_child_optional("payable")) {
                    payable = node.second.get<bool>("payable");
                }
                auto inputNodes = node.second.get_child("inputs");
                for (auto inputNode : inputNodes) {
                    string type = inputNode.second.get<string>("type");
                    tds.push_back(TypeDef(type));
                }
                auto orderNode = node.second.get_child("order");
                if (orderNode.empty())
                    continue;
                for (auto &orderVal : orderNode) {
                    int order = orderVal.second.get_value<int>();
                    this->fds[lengthOrder + lengthNotOrder + order] = FuncDef(name, tds, payable);
                    /* test Order */
                    std::cout << "### A2 Order! Function:" << name << " : " << lengthOrder+lengthNotOrder+order << endl;
                }
            }
        };
        /* test A2 */
        std::cout << "A2: FDS size: " << lengthOrder << std::endl;
        for (size_t i = lengthOrder+lengthNotOrder; i < lengthOrder+lengthNotOrder + lengthOrder + 1; ++i) {
            std::cout << "A2: FDS[" << i << "]:" << this->fds[i].name << std::endl;
        }

    } else if (orderS == "AB" && isMain) {
        std::cout << "################################## AB ##################################" << std::endl;
        this->fds.resize(lengthOrder + lengthNotOrder);
        /* | ----- lengthOrder ----- | ---- lengthNotOrder ---- | */

        // A1: Add a function with an order field to the fds
        for (auto node : root) {
            vector<TypeDef> tds;
            string type = node.second.get<string>("type");
            string constant = "false";
            bool payable = false;
            // constant
            if (node.second.get_child_optional("constant")) {
                constant = node.second.get<string>("constant");
            }
            // fallback
            if (type == "fallback" && node.second.get_child_optional("order")) {
                if (node.second.get_child_optional("payable")) {
                    payable = node.second.get<bool>("payable");
                }
                auto orderNode = node.second.get_child("order");
                if (orderNode.empty())
                    continue;
                for (auto &orderVal : orderNode) {
                    int order = orderVal.second.get_value<int>();
                    this->fds[order] = FuncDef("fallback", tds, payable);
                    /* test */
                    std::cout << "### A1 Order! " << "fallback" << " : " << order << std::endl << "*****\n";
                }
            }
            // function -- At that time, A1 doesn't need a constructor
            //      if ((type == "constructor" || type == "function") && constant == "false" && node.second.get_child_optional("order")) {
            if (type == "function" && constant == "false" && node.second.get_child_optional("order")) {
                string name = type == "constructor" ? "" : node.second.get<string>("name");
                if (node.second.get_child_optional("payable")) {
                    payable = node.second.get<bool>("payable");
                }

                auto inputNodes = node.second.get_child("inputs");
                for (auto inputNode : inputNodes) {
                    string type = inputNode.second.get<string>("type");
                    tds.push_back(TypeDef(type));
                }

                auto orderNode = node.second.get_child("order");
                if (orderNode.empty())
                    continue ;
                for (auto &orderVal: orderNode) {
                    int order = orderVal.second.get_value<int>();
                    this->fds[order] = FuncDef(name, tds, payable);
                    /* test */
                    std::cout << "### A1 Order! Function: " << name << " : " << order << std::endl << "*****\n";
                }
            }
        };
        /* test A1 */
        //std::cout << "after : " << this->fds.size() << std::endl;
        std::cout << "A1: FDS size: " << lengthOrder << std::endl;
        for (size_t i = 0; i < lengthOrder; ++i) {
            std::cout << "A1: FDS[" << i << "]: " << this->fds[i].name << std::endl;
        }

        /* ----------------------------------------------------------------- */

        int indexB = 0;
        // B: Add a function to the fds that meets the inclusion criteria, but doesn't have an order field
        for (auto node : root) {
            if (indexB >= lengthNotOrder) break;
            vector<TypeDef> tds;
            string type = node.second.get<string>("type");
            string constant = "false";
            bool payable = false;
            // constant
            if (node.second.get_child_optional("constant")) {
                constant = node.second.get<string>("constant");
            }
            // fallback
            if (type == "fallback" && !(node.second.get_child_optional("order"))) {
                if (node.second.get_child_optional("payable")) {  // 获取payable状态
                    payable = node.second.get<bool>("payable");
                }
                this->fds[lengthOrder + indexB++] = FuncDef("fallback", tds, payable);
                /* test */
                std::cout << "*** Here, B NotOrder! Fallback" << " : " << lengthOrder + indexB -1 << std::endl;
            }
            // function
            if ((type == "constructor" || type == "function") && constant == "false" && !(node.second.get_child_optional("order"))) {
                string name = type == "constructor" ? "" : node.second.get<string>("name");
                if (node.second.get_child_optional("payable")) {
                    payable = node.second.get<bool>("payable");
                }
                auto inputNodes = node.second.get_child("inputs");
                for (auto inputNode : inputNodes) {
                    string type = inputNode.second.get<string>("type");
                    tds.push_back(TypeDef(type));
                }
                this->fds[lengthOrder + indexB++] = FuncDef(name, tds, payable);
                /* test */
                std::cout << "*** Here, B NotOrder! Function" << name << " : " << lengthOrder + indexB -1 << std::endl;
            }
        };
        /* test B */
        std::cout << "B: FDS size:" << lengthNotOrder << std::endl;
        for (size_t i = lengthOrder; i < lengthOrder + lengthNotOrder; ++i){
            std::cout << "B: FDS[" << i << "]:" << this->fds[i].name << std::endl;
        }


    } else if (orderS =="BA" && isMain) {
        std::cout << "################################## BA ##################################" << std::endl;
        this->fds.resize(lengthNotOrder + lengthOrder +1);  // +1: add the number of constructor from A
        /* | ----- lengthNotOrder ----- | ---- lengthOrder +1 ---- | */

        int indexB = 0;
        /* Differ from 'ABA' and 'AB', the implementation of B in 'BA' needs to be modified by removing the beginning lengthOrder */
        // B: Add a function to the fds that meets the inclusion criteria, but doesn't have an order field
        for (auto node : root) {
            if (indexB >= lengthNotOrder) break;
            vector<TypeDef> tds;
            string type = node.second.get<string>("type");
            string constant = "false";
            bool payable = false;
            // constant
            if (node.second.get_child_optional("constant")) {
                constant = node.second.get<string>("constant");
            }
            // fallback
            if (type == "fallback" && !(node.second.get_child_optional("order"))) {
                if (node.second.get_child_optional("payable")) {  // 获取payable状态
                    payable = node.second.get<bool>("payable");
                }
                this->fds[indexB++] = FuncDef("fallback", tds, payable);
                /* test */
                std::cout << "*** Here, B NotOrder! Fallback" << " : " << indexB -1 << std::endl;
            }
            // function
            if ((type == "constructor" || type == "function") && constant == "false" && !(node.second.get_child_optional("order"))) {
                string name = type == "constructor" ? "" : node.second.get<string>("name");
                if (node.second.get_child_optional("payable")) {
                    payable = node.second.get<bool>("payable");
                }
                auto inputNodes = node.second.get_child("inputs");
                for (auto inputNode : inputNodes) {
                    string type = inputNode.second.get<string>("type");
                    tds.push_back(TypeDef(type));
                }
                this->fds[indexB++] = FuncDef(name, tds, payable);
                /* test */
                std::cout << "*** Here, B NotOrder! Function" << name << " : " << indexB -1 << std::endl;
            }
        };
        /* test B */
        std::cout << "B: FDS size:" << lengthNotOrder << std::endl;
        for (size_t i = 0; i < lengthNotOrder; ++i){
            std::cout << "B: FDS[" << i << "]:" << this->fds[i].name << std::endl;
        }

        /* ----------------------------------------------------------------- */

        /* A2: Append sequences with an order field that meet the join criteria to the end */
        /*    lengthNotOrder  -->  lengthNotOrder + lengthOrder + 1 */
        /* A2: length = lengthOrder + 1. order range in: [0, lengthOrder] */
        for (auto node : root) {
            vector<TypeDef> tds;
            string type = node.second.get<string>("type");
            string constant = "false";
            bool payable = false;
            // constant
            if (node.second.get_child_optional("constant")) {
                constant = node.second.get<string>("constant");
            }
            // fallback
            if (type == "fallback" && node.second.get_child_optional("order")) {
                if (node.second.get_child_optional("payable")) {
                    payable = node.second.get<bool>("payable");
                }
                auto orderNode = node.second.get_child("order");
                if (orderNode.empty())
                    continue;
                for (auto &orderVal : orderNode) {
                    int order = orderVal.second.get_value<int>();
                    this->fds[lengthNotOrder + order] = FuncDef("fallback", tds, payable);
                    /* test Order */
                    cout << "### A2 Order! Fallback : " << lengthNotOrder+order << endl;
                }
            }
            // function
            if ((type == "constructor" || type == "function") && constant == "false" && node.second.get_child_optional("order")) {
                string name = type == "constructor" ? "" : node.second.get<string>("name");
                if (node.second.get_child_optional("payable")) {
                    payable = node.second.get<bool>("payable");
                }
                auto inputNodes = node.second.get_child("inputs");
                for (auto inputNode : inputNodes) {
                    string type = inputNode.second.get<string>("type");
                    tds.push_back(TypeDef(type));
                }
                auto orderNode = node.second.get_child("order");
                if (orderNode.empty())
                    continue;
                for (auto &orderVal : orderNode) {
                    int order = orderVal.second.get_value<int>();
                    this->fds[lengthNotOrder + order] = FuncDef(name, tds, payable);
                    /* test Order */
                    std::cout << "### A2 Order! Function:" << name << " : " << lengthNotOrder+order << endl;
                }
            }
        };
        /* test A2 */
        std::cout << "A2: FDS size: " << lengthOrder << std::endl;
        for (size_t i = lengthNotOrder; i < lengthNotOrder + lengthOrder + 1; ++i) {
            std::cout << "A2: FDS[" << i << "]:" << this->fds[i].name << std::endl;
        }


    } else { // If there are no matches, the default is dropped
        for (auto node : root) {
            vector<TypeDef> tds;
            string type = node.second.get<string>("type");
            string constant = "false";
            bool payable = false;
            if (node.second.get_child_optional("constant")) {
                constant = node.second.get<string>("constant");
            }
            if (type == "fallback") {
                if (node.second.get_child_optional("payable")) {
                    payable = node.second.get<bool>("payable");
                }
                this->fds.push_back(FuncDef("fallback", tds, payable));
            }
            if ((type == "constructor" || type == "function") && constant == "false") {
                auto inputNodes = node.second.get_child("inputs");
                string name = type == "constructor" ? "" : node.second.get<string>("name");
                if (node.second.get_child_optional("payable")) {
                    payable = node.second.get<bool>("payable");
                }
                for (auto inputNode : inputNodes) {
                    string type = inputNode.second.get<string>("type");
                    tds.push_back(TypeDef(type));
                }
                this->fds.push_back(FuncDef(name, tds, payable));
            }
        };
        return;
    }

    cout << "********************************************************" << endl;
    cout << "--------------------------------------------------------" << endl;


    size_t flagHaveConst = 0;


    if (orderS == "ABA" && isMain) {
        /* total sequence */
        cout << "Testing Total Sequence!" << endl;
        for (size_t i = 0; i < lengthOrder + lengthNotOrder + lengthOrder +1; ++i) {
            std::cout << "Total FDS[" << i << "]:" << this->fds[i].name << std::endl;
        }
        /* to fix the position of Constructor */
        for (size_t i = 0; i < lengthOrder + lengthNotOrder + lengthOrder -1; ++i){  // 2 less place can be the constructor
            if (this->fds[i].name == "")
                flagHaveConst = 1;
        }
        if (flagHaveConst) {
            // Find the location of the constructor
            int constructorIndex = -1;
            FuncDef constructorFuncDef("", vector<TypeDef>(), false);
            for (size_t i = 0; i < this->fds.size()-1; ++i) {
                if (this->fds[i].name == "") {
                    constructorIndex = i;
                    constructorFuncDef = this->fds[i];
                    break;
                }
            }
            // If constructor is found
            if (constructorIndex != -1) {
                // Moves all functions after the constructor one bit forward
                for (size_t i = constructorIndex; i < this->fds.size() - 2; ++i) {  //
                    this->fds[i] = this->fds[i + 1];
                }
                // Place the constructor last bit
                this->fds[this->fds.size() - 1] = constructorFuncDef;
            }
            /* View total sequence */
            cout << "In **ABA**, After Moving the CONSTRUCTOR, the FDS!" << endl;
            for (size_t i = 0; i < lengthOrder + lengthNotOrder + lengthOrder+1; ++i) {
                std::cout << "FDS[" << i << "]:" << this->fds[i].name << std::endl;
            }
        } else {
            cout << "Constructor's position is good!" << endl;
        }

    } else if (orderS == "AB" && isMain) {
        /* total sequence */
        cout << "Testing Total Sequence!" << endl;
        for (size_t i = 0; i < lengthOrder + lengthNotOrder; ++i) {
            std::cout << "Total FDS[" << i << "]:" << this->fds[i].name << std::endl;
        }
        /* to fix the position of Constructor */
        for (size_t i = 0; i < lengthOrder + lengthNotOrder -2; ++i){  // 2 less place can be the constructor
            if (this->fds[i].name == "")
                flagHaveConst = 1;
        }
        if (flagHaveConst) {
            // Find the location of the constructor
            int constructorIndex = -1;
            FuncDef constructorFuncDef("", vector<TypeDef>(), false);
            for (size_t i = 0; i < this->fds.size()-1; ++i) {
                if (this->fds[i].name == "") {
                    constructorIndex = i;
                    constructorFuncDef = this->fds[i];
                    break;
                }
            }
            // If constructor is found
            if (constructorIndex != -1) {
                // Moves all functions after the constructor one bit forward
                for (size_t i = constructorIndex; i < this->fds.size() - 2; ++i) {  //
                    this->fds[i] = this->fds[i + 1];
                }
                // Place the constructor last bit
                this->fds[this->fds.size() - 1] = constructorFuncDef;
            }
            /* View total sequence */
            cout << "In **AB**, After Moving the CONSTRUCTOR, the FDS!" << endl;
            for (size_t i = 0; i < lengthOrder + lengthNotOrder; ++i) {
                std::cout << "FDS[" << i << "]:" << this->fds[i].name << std::endl;
            }
        } else {
            cout << "Constructor's position is good!" << endl;
        }

    } else if (orderS == "BA" && isMain) {
        /* total sequence */
        cout << "Testing Total Sequence!" << endl;
        for (size_t i = 0; i < lengthNotOrder + lengthOrder +1; ++i) {
            std::cout << "Total FDS[" << i << "]:" << this->fds[i].name << std::endl;
        }
        /* to fix the position of Constructor */
        for (size_t i = 0; i < lengthNotOrder + lengthOrder -1; ++i){  // 2 less place can be the constructor
            if (this->fds[i].name == "")
                flagHaveConst = 1;
        }
        if (flagHaveConst) {
            // Find the location of the constructor
            int constructorIndex = -1;
            FuncDef constructorFuncDef("", vector<TypeDef>(), false);
            for (size_t i = 0; i < this->fds.size()-1; ++i) {
                if (this->fds[i].name == "") {
                    constructorIndex = i;
                    constructorFuncDef = this->fds[i];
                    break;
                }
            }

            // If constructor is found
            if (constructorIndex != -1) {
                // Moves all functions after the constructor one bit forward
                for (size_t i = constructorIndex; i < this->fds.size() - 2; ++i) {  //
                    this->fds[i] = this->fds[i + 1];
                }
                // Place the constructor last bit
                this->fds[this->fds.size() - 1] = constructorFuncDef;
            }
            /* View total sequence */
            cout << "In **BA**, After Moving the CONSTRUCTOR, the FDS!" << endl;
            for (size_t i = 0; i < lengthNotOrder + lengthOrder +1; ++i) {
                std::cout << "FDS[" << i << "]:" << this->fds[i].name << std::endl;
            }
        } else {
            cout << "Constructor's position is good!" << endl;
        }

    }
//    if (flagHaveConst) {
//        // Find the location of the constructor
//        int constructorIndex = -1;
//        FuncDef constructorFuncDef("", vector<TypeDef>(), false);
//        for (size_t i = 0; i < this->fds.size()-1; ++i) {
//            if (this->fds[i].name == "") {
//                constructorIndex = i;
//                constructorFuncDef = this->fds[i];
//                break;
//            }
//        }
//        // If constructor is found
//        if (constructorIndex != -1) {
//            // Moves all functions after the constructor one bit forward
//            for (size_t i = constructorIndex; i < this->fds.size() - 2; ++i) {  //
//                this->fds[i] = this->fds[i + 1];
//            }
//            // Place the constructor last bit
//            this->fds[this->fds.size() - 1] = constructorFuncDef;
//        }
//        /* View total sequence */
//        if (orderS == "ABA" && isMain) {
//            cout << "In **ABA**, After Moving the CONSTRUCTOR, the FDS!" << endl;
//            for (size_t i = 0; i < lengthOrder + lengthNotOrder + lengthOrder+1; ++i) {
//                std::cout << "FDS[" << i << "]:" << this->fds[i].name << std::endl;
//            }
//        } else if (orderS == "AB" && isMain) {
//            cout << "In **AB**, After Moving the CONSTRUCTOR, the FDS!" << endl;
//            for (size_t i = 0; i < lengthOrder + lengthNotOrder; ++i) {
//                std::cout << "FDS[" << i << "]:" << this->fds[i].name << std::endl;
//            }
//        } else if (orderS == "BA" && isMain) {
//            cout << "In **BA**, After Moving the CONSTRUCTOR, the FDS!" << endl;
//            for (size_t i = 0; i < lengthNotOrder + lengthOrder +1; ++i) {
//                std::cout << "FDS[" << i << "]:" << this->fds[i].name << std::endl;
//            }
//        }
//
//    } else {
//        cout << "Constructor's position is good!" << endl;
//    }
}



  
  bytes ContractABI::encodeConstructor() {
    auto it = find_if(fds.begin(), fds.end(), [](FuncDef fd) { return fd.name == "";});
    if (it != fds.end()) return encodeTuple((*it).tds);
    return bytes(0, 0);
  }
//  bytes ContractABI::encodeConstructor(vector<TypeDef> tds) {
//    auto it = find_if(fds.begin(), fds.end(), [](FuncDef fd) { return fd.name == "";});
//    if (it != fds.end()) return encodeTuple((*it).tds);
//    return bytes(0, 0);
//  }
  
  bool ContractABI::isPayable(string name) {
    for (auto fd : fds) {
      if (fd.name == name) return fd.payable;
    }
    return false;
  }
  
  vector<bytes> ContractABI::encodeFunctions() {
    vector<bytes> ret;
    for (auto fd : fds) {
      if (fd.name != "") {
        bytes selector = functionSelector(fd.name /* name */, fd.tds /* type defs */);
        bytes data = encodeTuple(fd.tds);
        selector.insert(selector.end(), data.begin(), data.end());
        ret.push_back(selector);
      }
    }
    return ret;
  }
  
  bytes ContractABI::functionSelector(string name, vector<TypeDef> tds) {
    vector<string> argTypes;
    transform(tds.begin(), tds.end(), back_inserter(argTypes), [](TypeDef td) {
      return td.fullname;
    });
    string signature = name + "(" + boost::algorithm::join(argTypes, ",") + ")";
    bytes fullSelector = sha3(signature).ref().toBytes();
    return bytes(fullSelector.begin(), fullSelector.begin() + 4);
  }

  bytes ContractABI::functionSelector(string name) {
      vector<string> argTypes;
      string signature = name + "(" + boost::algorithm::join(argTypes, ",") + ")";
      bytes fullSelector = sha3(signature).ref().toBytes();
      return bytes(fullSelector.begin(), fullSelector.begin() + 4);
  }
  
  bytes ContractABI::encodeTuple(vector<TypeDef> tds) {
    bytes ret;
    /* Payload */
    bytes payload;
    vector<int> dataOffset = {0};
    for (auto td : tds) {
      if (td.isDynamic || td.isDynamicArray || td.isSubDynamicArray) {
        bytes data;
        switch (td.dimensions.size()) {
          case 0: {
            data = encodeSingle(td.dt);
            break;
          }
          case 1: {
            data = encodeArray(td.dts, td.isDynamicArray);
            break;
          }
          case 2: {
            data = encode2DArray(td.dtss, td.isDynamicArray, td.isSubDynamicArray);
            break;
          }
        }
        dataOffset.push_back(dataOffset.back() + data.size());
        payload.insert(payload.end(), data.begin(), data.end());
      }
    }
    /* Calculate offset */
    u256 headerOffset = 0;
    for (auto td : tds) {
      if (td.isDynamic || td.isDynamicArray || td.isSubDynamicArray) {
        headerOffset += 32;
      } else {
        switch (td.dimensions.size()) {
          case 0: {
            headerOffset += encodeSingle(td.dt).size();
            break;
          }
          case 1: {
            headerOffset += encodeArray(td.dts, td.isDynamicArray).size();
            break;
          }
          case 2: {
            headerOffset += encode2DArray(td.dtss, td.isDynamicArray, td.isSubDynamicArray).size();
            break;
          }
        }
      }
    }
    bytes header;
    int dynamicCount = 0;
    for (auto td : tds) {
      /* Dynamic in head */
      if (td.isDynamic || td.isDynamicArray || td.isSubDynamicArray) {
        u256 offset = headerOffset + dataOffset[dynamicCount];
        /* Convert to byte */
        for (int i = 0; i < 32; i += 1) {
          byte b = (byte) (offset >> ((32 - i - 1) * 8)) & 0xFF;
          header.push_back(b);
        }
        dynamicCount ++;
      } else {
        /* static in head */
        bytes data;
        switch (td.dimensions.size()) {
          case 0: {
            data = encodeSingle(td.dt);
            break;
          }
          case 1: {
            data = encodeArray(td.dts, td.isDynamicArray);
            break;
          }
          case 2: {
            data = encode2DArray(td.dtss, td.isDynamicArray, td.isSubDynamicArray);
            break;
          }
        }
        header.insert(header.end(), data.begin(), data.end());
      }
    }
    /* Head + Payload */
    ret.insert(ret.end(), header.begin(), header.end());
    ret.insert(ret.end(), payload.begin(), payload.end());
    return ret;
  }
  
  bytes ContractABI::encode2DArray(vector<vector<DataType>> dtss, bool isDynamicArray, bool isSubDynamic) {
    bytes ret;
    if (isDynamicArray) {
      bytes payload;
      bytes header;
      u256 numElem = dtss.size();
      if (isSubDynamic) {
        /* Need Offset*/
        vector<int> dataOffset = {0};
        for (auto dts : dtss) {
          bytes data = encodeArray(dts, isSubDynamic);
          dataOffset.push_back(dataOffset.back() + data.size());
          payload.insert(payload.end(), data.begin(), data.end());
        }
        /* Count */
        for (int i = 0; i < 32; i += 1) {
          byte b = (byte) (numElem >> ((32 - i - 1) * 8)) & 0xFF;
          header.push_back(b);
        }
        for (int i = 0; i < numElem; i += 1) {
          u256 headerOffset =  32 * numElem + dataOffset[i];
          for (int i = 0; i < 32; i += 1) {
            byte b = (byte) (headerOffset >> ((32 - i - 1) * 8)) & 0xFF;
            header.push_back(b);
          }
        }
      } else {
        /* Count */
        for (int i = 0; i < 32; i += 1) {
          byte b = (byte) (numElem >> ((32 - i - 1) * 8)) & 0xFF;
          header.push_back(b);
        }
        for (auto dts : dtss) {
          bytes data = encodeArray(dts, isSubDynamic);
          payload.insert(payload.end(), data.begin(), data.end());
        }
      }
      ret.insert(ret.end(), header.begin(), header.end());
      ret.insert(ret.end(), payload.begin(), payload.end());
      return ret;
    }
    for (auto dts : dtss) {
      bytes data = encodeArray(dts, isSubDynamic);
      ret.insert(ret.end(), data.begin(), data.end());
    }
    return ret;
  }
  
  bytes ContractABI::encodeArray(vector<DataType> dts, bool isDynamicArray) {
    bytes ret;
    /* T[] */
    if (isDynamicArray) {
      /* Calculate header and payload */
      bytes payload;
      bytes header;
      u256 numElem = dts.size();
      if (dts[0].isDynamic) {
        /* If element is dynamic then needs offset */
        vector<int> dataOffset = {0};
        for (auto dt : dts) {
          bytes data = encodeSingle(dt);
          dataOffset.push_back(dataOffset.back() + data.size());
          payload.insert(payload.end(), data.begin(), data.end());
        }
        /* Count */
        for (int i = 0; i < 32; i += 1) {
          byte b = (byte) (numElem >> ((32 - i - 1) * 8)) & 0xFF;
          header.push_back(b);
        }
        /* Offset */
        for (int i = 0; i < numElem; i += 1) {
          u256 headerOffset =  32 * numElem + dataOffset[i];
          for (int i = 0; i < 32; i += 1) {
            byte b = (byte) (headerOffset >> ((32 - i - 1) * 8)) & 0xFF;
            header.push_back(b);
          }
        }
      } else {
        /* Do not need offset, count them */
        for (int i = 0; i < 32; i += 1) {
          byte b = (byte) (numElem >> ((32 - i - 1) * 8)) & 0xFF;
          header.push_back(b);
        }
        for (auto dt : dts) {
          bytes data = encodeSingle(dt);
          payload.insert(payload.end(), data.begin(), data.end());
        }
      }
      ret.insert(ret.end(), header.begin(), header.end());
      ret.insert(ret.end(), payload.begin(), payload.end());
      return ret;
    }
    /* T[k] */
    for (auto dt : dts) {
      bytes data = encodeSingle(dt);
      ret.insert(ret.end(), data.begin(), data.end());
    }
    return ret;
  }
  
  bytes ContractABI::encodeSingle(DataType dt) {
    bytes ret;
    bytes payload = dt.payload();
    if (dt.isDynamic) {
      /* Concat len and data */
      bytes header = dt.header();
      ret.insert(ret.end(), header.begin(), header.end());
      ret.insert(ret.end(), payload.begin(), payload.end());
      return ret;
    }
    ret.insert(ret.end(), payload.begin(), payload.end());
    return ret;
  }
  
  DataType::DataType(bytes value, bool padLeft, bool isDynamic) {
    this->value = value;
    this->padLeft = padLeft;
    this->isDynamic = isDynamic;
  }
  
  bytes DataType::header() {
    u256 size = this->value.size();
    bytes ret;
    for (int i = 0; i < 32; i += 1) {
      byte b = (byte) (size >> ((32 - i - 1) * 8)) & 0xFF;
      ret.push_back(b);
    }
    return ret;
  }
  
  bytes DataType::payload() {
    auto paddingLeft = [this](double toLen) {
      bytes ret(toLen - this->value.size(), 0);
      ret.insert(ret.end(), this->value.begin(), this->value.end());
      return ret;
    };
    auto paddingRight = [this](double toLen) {
      bytes ret;
      ret.insert(ret.end(), this->value.begin(), this->value.end());
      while(ret.size() < toLen) ret.push_back(0);
      return ret;
    };
    if (this->value.size() > 32) {
      if (!this->isDynamic) throw "Size of static <= 32 bytes";
      int valueSize = this->value.size();
      int finalSize = valueSize % 32 == 0 ? valueSize : (valueSize / 32 + 1) * 32;
      if (this->padLeft) return paddingLeft(finalSize);
      return paddingRight(finalSize);
    }
    if (this->padLeft) return paddingLeft(32);
    return paddingRight(32);
  }
  
  string TypeDef::toRealname(string name) {
    string fullType = toFullname(name);
    string searchPatterns[2] = {"address[", "bool["};
    string replaceCandidates[2] = {"uint160", "uint8"};
    for (int i = 0; i < 2; i += 1) {
      string pattern = searchPatterns[i];
      string candidate = replaceCandidates[i];
      if (boost::starts_with(fullType, pattern))
        return candidate + fullType.substr(pattern.length() - 1);
      if (fullType == pattern.substr(0, pattern.length() - 1)) return candidate;
    }
    return fullType;
  }
  
  string TypeDef::toFullname(string name) {
    string searchPatterns[4] = {"int[", "uint[", "fixed[", "ufixed["};
    string replaceCandidates[4] = {"int256", "uint256", "fixed128x128", "ufixed128x128"};
    for (int i = 0; i < 4; i += 1) {
      string pattern = searchPatterns[i];
      string candidate = replaceCandidates[i];
      if (boost::starts_with(name, pattern))
        return candidate + name.substr(pattern.length() - 1);
      if (name == pattern.substr(0, pattern.length() - 1)) return candidate;
    }
    return name;
  }
  
  vector<int> TypeDef::extractDimension(string name) {
    vector<int> ret;
    smatch sm;
    regex_match(name, sm, regex("[a-z]+[0-9]*\\[(\\d*)\\]\\[(\\d*)\\]"));
    if (sm.size() == 3) {
      /* Two dimension array */
      ret.push_back(sm[1] == "" ? 0 : stoi(sm[1]));
      ret.push_back(sm[2] == "" ? 0 : stoi(sm[2]));
      return ret;
    }
    regex_match(name, sm, regex("[a-z]+[0-9]*\\[(\\d*)\\]"));
    if (sm.size() == 2) {
      /* One dimension array */
      ret.push_back(sm[1] == "" ? 0 : stoi(sm[1]));
      return ret;
    }
    return ret;
  }
  
  void TypeDef::addValue(vector<vector<bytes>> vss) {
    if (this->dimensions.size() != 2) throw "Invalid dimension";;
    for (auto vs : vss) {
      vector<DataType> dts;
      for (auto v : vs) {
        dts.push_back(DataType(v, this->padLeft, this->isDynamic));
      }
      this->dtss.push_back(dts);
    }
  }
  
  void TypeDef::addValue(vector<bytes> vs) {
    if (this->dimensions.size() != 1) throw "Invalid dimension";
    for (auto v : vs) {
      this->dts.push_back(DataType(v, this->padLeft, this->isDynamic));
    }
  }
  
  void TypeDef::addValue(bytes v) {
    if (this->dimensions.size()) throw "Invalid dimension";
    this->dt = DataType(v, this->padLeft, this->isDynamic);
  }
  
  TypeDef::TypeDef(string name) {
    this->name = name;
    this->fullname = toFullname(name);
    this->realname = toRealname(name);
    this->dimensions = extractDimension(name);
    this->padLeft = !boost::starts_with(this->fullname, "bytes") && !boost::starts_with(this->fullname, "string");
    int numDimension = this->dimensions.size();
    if (!numDimension) {
      this->isDynamic = this->fullname == "string" || this->name == "bytes";
      this->isDynamicArray = false;
      this->isSubDynamicArray = false;
    } else if (numDimension == 1) {
      this->isDynamic = boost::starts_with(this->fullname, "string[")
      || boost::starts_with(this->fullname, "bytes[");
      this->isDynamicArray = this->dimensions[0] == 0;
      this->isSubDynamicArray = false;
    } else {
      this->isDynamic = boost::starts_with(this->fullname, "string[")
      || boost::starts_with(this->fullname, "bytes[");
      this->isDynamicArray = this->dimensions[0] == 0;
      this->isSubDynamicArray = this->dimensions[1] == 0;
    }
  }

  /* new */
  bytes ContractABI::specialTestcase(const string& jsonData) {
    /*
     * `input` jsonData: the concrete data from json, instead of json Path.
     * The format of jsonData just like that:
//## Attention please! The values given below for the specific parameters and the specific name field in the parameters are just an example ##/
```json
[
    {
        "id": 1,
        "type": "constructor",
        "balance": "0x_",
        "sender": "0x_",
        "block_number": "0x_",
        "timestamp": "0x_",
        "params": [
          {
            "name": "[parameter_name]",
            "type": "[concrete_parameter_type]",
            "value": "[concrete_parameter_values]"
          }
        ]
    },
    {
        "id": 2,
        "type": "function",
        "name": "transfer",
        "balance": "0x_",
        "sender": "0x_",
        "block_number": "0x_",
        "timestamp": "0x_",
        "params": [
          {
            "name": "params1_name_in_here",
            "type": "uint256",
            "value": "100"
          },
          {
            "name": "param2_name_in_here",
            "type": "address",
            "value": "0x_"
          },
          {
            "name": "params3_name_in_here",
            "type": "uint256[]",
            "value": ["1", "2", "3"]
          }
        ]
    },
    ...
]
```
     * `balance`: The Sender's account balance, 12 byte
     *   - The balance of Sender's account, before exec the Transaction
     ///## 需要注意！！这里的balance是发送交易者的余额，是开始执行交易之前的余额信息。并不是合约的余额! ##///
     * `sender`: Address of sender, 20 byte
     * `block_number`: Block number of Contract, 8 byte
     * `timestamp`: that timestamp, 8 byte
     */
    // 计算 randomTestcase 函数生成的字节数据的长度
    bytes randomData = randomTestcase();
    size_t dataLength = randomData.size();

    // 分配相同长度的字节数据
    bytes data(dataLength, 0);
    // 给data的前32字节预留空间
    bytes dynamicLengths(32, 0);

    // 解析 JSON 数据
    pt::ptree root;
    stringstream ss(jsonData);
    pt::read_json(ss, root);

    // 解析 JSON 数据并填充字节数据
    int offset = 32; // 预留出给动态数组长度的32位空间，开始就是[32]也就是第33位。
    /* 0-31: 动态数组长度 (32 byte) */

    // For each one function call, in the all Function Call Sequence from jsonData.
    for (auto& item : root) {
      string type = item.second.get<string>("type");
      string balance = item.second.get<string>("balance");
      string sender = item.second.get<string>("sender");
      string blockNumber = item.second.get<string>("block_number");
      string timestamp = item.second.get<string>("timestamp");

      // 填充 balance (in block) - The balance of Sender's account, before exec the Transaction
      /* 32-43: balance (12 byte) */
      bytes balanceBytes = fromHex(balance);
      copy(balanceBytes.begin(), balanceBytes.end(), data.begin() + offset);
      offset += 12;

      // 填充 sender
      /* 44-63: sender (20 byte) */
      bytes senderBytes = fromHex(sender);
      copy(senderBytes.begin(), senderBytes.end(), data.begin() + offset);
      offset += 20;

      // 填充 block number 和 timestamp
      /* 64-71: block_number (8 byte) */
      /* 72-79: timestamp (8 byte) */
      bytes blockNumberBytes = fromHex(blockNumber);
      bytes timestampBytes = fromHex(timestamp);
      copy(blockNumberBytes.begin(), blockNumberBytes.end(), data.begin() + offset);
      offset += 8;
      copy(timestampBytes.begin(), timestampBytes.end(), data.begin() + offset);
      offset += 8;

      // 这里需要给16字节填充0，这里是预留空间，不需要用到。
      /* 80-95: padding 0 as reserved space (16 byte) */
      offset += 16;

      // 填充函数参数
      /* 96-data.end(): params.  */
      /*
       * the size of the dynamic array needs to be specified by bits 0-31 of the 'data' of the function input.
       */
      auto params = item.second.get_child("params");
      int dynamicIndex = 0;
      for (auto& param : params) {  // 对分别的每一个参数项，进行前32字节位的动态数组长度划定
        string paramType = param.second.get<string>("type");
        if (paramType.find("[]") != string::npos) {
          // 如果参数是动态数组，计算其长度并存储在dynamicLengths数组中
          auto values = param.second.get_child("value");
          if (paramType.find("[][]") != string::npos) {
            // 处理二维数组
            int outerLength = values.size();
            dynamicLengths[dynamicIndex++] = outerLength;
            if (outerLength > 0) {
              int innerLength = values.begin()->second.size();
              dynamicLengths[dynamicIndex++] = innerLength;
            }
          }
          else {
            // 处理一维数组
            int length = values.size();
            dynamicLengths[dynamicIndex++] = length;
          }
        }
        stringstream paramsStream;
        boost::property_tree::write_json(paramsStream, params);
        auto paramsString = paramsStream.str();
        bytes paramsBytes = fromHex(paramsString);
        copy(paramsBytes.begin(), paramsBytes.end(), data.begin() + offset);
        offset += paramsBytes.size();
      }
    }

    // 将动态数组长度复制到data数组的前32个字节
    copy(dynamicLengths.begin(), dynamicLengths.end(), data.begin());

    return data;
  }
}
