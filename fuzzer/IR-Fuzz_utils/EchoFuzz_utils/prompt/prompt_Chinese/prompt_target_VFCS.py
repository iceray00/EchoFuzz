import os, sys

sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__))))

from import_VFCS import introduction_VFCS

intro_VFCS = f"""
{introduction_VFCS}

现在，这里会展示出所需要检测的合约源代码：
"""

# will show: {source_code} in here


LLM_generate_VFCS_1 = """

除此之外，这里有一些信息，需要你得知：
1、Q&A_1：
- 这是我第一次使用大语言模型LLM提问时给出的结果：
**Q&A_1:**
"
```md
{}
```
"
"""

LLM_generate_VFCS_2 = """
2、Q&A_2：
这里，给出了我第二次使用大语言模型LLM提问时给出的结果：
**Q&A_2:**

```md
{}
```
"""

staticAnalysis_report_abi = """
这里，使用slither静态分析工具，对当前的合约进行静态分析，将其得到的静态分析报告和abi信息展示在这：

{}
"""

output_VFCS_format = """
[mainContractName]_id:
`function_1 -> function_2 -> function_3`(-> constructor)
"""

VFCS_exmple = """
假设主合约的名称为`mainContractName`，（注意在同一份source code中可能存在多个合约，但是主合约有且仅有一个，不能存在多个！在输出时只输出主合约有关的VFCS！function不能是判断为是被private修饰的函数！）那么输出的格式就应该如下所示：

output:
```
mainContractName_1:
`function_1 -> function_2 -> function_3`(-> constructor)

mainContractName_2:
`function_4 -> function_1 -> function_1 -> function_3`(-> constructor)

mainContractName_3:
`function_2 -> function_1 -> function_2 -> function_4`(-> constructor)
```

"""

LLM_generate_VFCS_target = f"""
上面就是完整的信息内容！
VFCS的格式：
{output_VFCS_format}

这里展示关于VFCS输出格式的样板：
{VFCS_exmple}

这里有几个需要注意、需要强调的点：
1、这里的`function_*`，不能是之前LLM给出的Q&A_1中判断为是被private修饰的函数，并且在输出时只能输出当前合约中有的函数不能输出别的合约中的函数！
2、这里的`function_*`，是被认为比较重要的函数、是用于构成VFCS的函数！
3、上面只是一个例子，是一个符合VFCS格式的示例。具体需要输出的内容，需要根据具体的合约源代码和给出的Q&A信息、静态分析报告给出！
4、所需要给出的VFCS一定是根据abi而来的！如果abi中没有给出的函数，即使很关键，也不能显式的出现在VFCS中！
5、你一定要看清abi中的内容，很多在合约中取决定性作用的函数（比如转账、调用其他合约、权限控制、关键信息判断等函数）有坑会在solidity中修饰为`private`类型。但是在abi中，private并不能展示出来！而我们所需要的VFCS，也不允许包含private修饰的语句！如果要显示这个函数，这应该显示离这个函数最近的，调用这个函数的父函数，并且这个父函数也不能是为‘private’修饰的！（可以参考之前LLM给出的Q&A）
6、function就直接输出函数名就可以了，不需要其他前后缀。function不能输出上述LLM给出的Q&A_2中比较重要的函数中没有的函数！
7、注意你需要判断出合约中的主合约是什么，并将source code中的主合约名称覆盖到上述格式中'mainContractName_1'的位置上，而不是直接输出mainContractName_1。
以下是一个例子便与你理解什么是主合约：
**主合约（Main Contract）**是指在一个多合约系统中，起主要作用、管理和协调其他合约的核心合约，并且一个合约里面有且仅有一个主合约，不会存在多个主合约。
pragma solidity ^0.4.0;
contract Governmental {{
  address public owner;
  address public lastInvestor;
  uint public jackpot = 1 ether;
  uint public lastInvestmentTimestamp;
  uint public ONE_MINUTE = 1 minutes;

  function Governmental() {{
    owner = msg.sender;
  //  if (msg.value<1 ether) throw;
  }}

  function invest() {{
    if (msg.value<jackpot/2) throw;
    lastInvestor = msg.sender;
    jackpot += msg.value/2;
    // <yes> <report> TIME_MANIPULATION
    lastInvestmentTimestamp = block.timestamp;
  }}

  function resetInvestment() {{
    if (block.timestamp < lastInvestmentTimestamp+ONE_MINUTE)
      throw;

    lastInvestor.send(jackpot);
    owner.send(this.balance-1 ether);

    lastInvestor = 0;
    jackpot = 1 ether;
    lastInvestmentTimestamp = 0;
  }}
}}

contract Attacker {{

  function attack(address target, uint count) {{
    if (0<=count && count<1023) {{
      this.attack.gas(msg.gas-2000)(target, count+1);
    }}
    else {{
      Governmental(target).resetInvestment();
    }}
  }}
}}
比如在上述这个合约例子中，存在两个合约一个是'Governmental'，一个是的'Attacker'，但因为'Governmental'起主要作用、管理和协调其他合约，所以'Governmental'是主合约，'Attacker'则不是主合约。因此在输出时mainContractName_1都要修改成Governmental_1。
8、VFCS可以是同样的函数进行了多次调用，一个VFCS中不一定是一个函数只能出现一次！ 比如play()->play()->getProfit()->play()->getProfit()这样的情况就是很好！这里既有相同函数的连续调用、也有不同函数的穿插调用，这个就是很好的样例！希望生成的VFCS长度不能无限长也不要输出过多的VFCS，最主要的要分析这个VFCS是不是确实会引发漏洞。
9、只输出主合约的VFCS。
10、只输出3个VFCS即可。


请你按照这个格式输出所有可能的VFCS！
**Please pay attention!! You only need to output the final VFCS result without any instructions!!**
output:

"""
