import os, sys

sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__))))

from import_VFCS import introduction_VFCS

intro_VFCS = f"""
你是一名智能合约漏洞审计专家，现在你将使用前面步骤得到的合约语义分析和静态验证的结果来生成一个易受攻击的函数调用序列（VFCS）。

{{introduction_VFCS}}

现在，这里会展示出所需要检测的合约源代码：
"""

# will show: {{source_code}} in here


LLM_generate_VFCS_1 = """

除此之外，这里有一些信息，需要你得知：
1、Q&A_1：
- 这是我第一次使用大语言模型LLM提问时给出的结果：
**Q&A_1:**
"
```md
{{}}
```
"
"""

LLM_generate_VFCS_2 = """
2、Q&A_2：
这里，给出了我第二次使用大语言模型LLM提问时给出的结果：
**Q&A_2:**

```md
{{}}
```
"""

staticAnalysis_report_abi = """
这里，使用slither静态分析工具，对当前的合约进行静态分析，将其得到的静态分析报告和abi信息展示在这：

{{}}
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
{{output_VFCS_format}}

这里展示关于VFCS输出格式的样板：
{{VFCS_exmple}}

这里有几个需要注意、需要强调的点：
1、这里的`function_*`，不能是之前LLM给出的Q&A_1中判断为是被private修饰的函数，并且在输出时只能输出当前合约中有的函数不能输出别的合约中的函数！
2、这里的`function_*`，是被认为关键函数、是用于构成VFCS的函数！
3、上面只是一个例子，是一个符合VFCS格式的示例。具体需要输出的内容，需要根据具体的合约源代码和给出的Q&A信息、静态分析报告给出！
4、所需要给出的VFCS一定是根据abi而来的！如果abi中没有给出的函数，即使很关键，也不能显式的出现在VFCS中！
5、你一定要看清abi中的内容，很多在合约中取决定性作用的函数（比如转账、调用其他合约、权限控制、关键信息判断等函数）有坑会在solidity中修饰为`private`类型。但是在abi中，private并不能展示出来！而我们所需要的VFCS，也不允许包含private修饰的语句！如果要显示这个函数，这应该显示离这个函数最近的，调用这个函数的父函数，并且这个父函数也不能是为‘private’修饰的！（可以参考之前LLM给出的Q&A）
6、注意你需要判断出合约中的主合约是什么，并将source code中的主合约名称覆盖到上述格式中'mainContractName_1'的位置上，而不是直接输出mainContractName_1。
以下是一个例子便与你理解什么是主合约：
**主合约（Main Contract）**是指在一个多合约系统中，起主要作用、管理和协调其他合约的核心合约，并且一个合约里面有且仅有一个主合约，不会存在多个主合约。
```solidity
pragma solidity ^0.4.24;
contract Crowdsale {{
  uint256 phase = 0;  // 0:Active, 1:Success
  uint256 goal;
  uint256 invested;
  address owner;
  mapping(address => uint256) invests;
  constructor() public{{
    goal = 100 ether;
    invested = 0;
    owner = msg.sender;
  }}
  function invest(uint256 donations) public payable {{
    if (invested<goal){{
      invests[msg.sender] += donations;
      invested += donations;
      phase = 0;
    }} else {{ phase = 1; }}
  }}
  function refund() public {{
    if (phase == 0) {{
      msg.sender.transfer(invests[msg.sender]);
      invested -= invests[msg.sender];
      invests[msg.sender] = 0;
  }} }}
  function withdraw() public {{
    if(phase == 1) {{
      bug();  // There exists a bug
      owner.transfer(invested);
    }} }}
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
```
比如在上述这个合约例子中，存在两个合约一个是'Crowdsale'，一个是的'Attacker'，但因为'Crowdsale'起主要作用、管理和协调其他合约，所以'Crowdsale'是主合约，'Attacker'则不是主合约。因此在输出时mainContractName_1都要修改成Crowdsale_1。
你要注意到：
1. 最有可能的VFCS是：invest -> invest -> withdraw。说明：
     a. invest与refund存在抵消关系，如果顺序调用的话，refund会使得变量invested的值被重置，这是相互抵消的函数！
     b. 要促使phase=1，invest需连续调用两次，在参数允许的情况下，invest -> invest就有机会将phase变为1。
     c. 漏洞触发：识别出withdraw的漏洞，并知道需在phase=1条件下才能够触发，因此形成：invest -> invest -> withdraw
     
2. 你会发现：VFCS可以是同样的函数进行了多次调用，并不一定只能出现一次！也有可能会存在既有相同函数的连续调用、也有不同函数的穿插调用！

3. 只输出主合约的VFCS。

4. 请输出3个最有可能的VFCS即可。


请你按照这个格式输出所有可能的VFCS！
**Please pay attention!! You only need to output the final VFCS result without any instructions!!**
output:

"""
