
import os, sys

sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__))))

from import_VFCS import introduction_VFCS

intro_VFCS = f"""
{introduction_VFCS}

Now, here is the contract source code to be analyzed:
"""

# will show: {source_code} in here

LLM_generate_VFCS_1 = """
Additionally, here is some information you need to know:
1. Q&A_1:
- This is the result of my first question using the large language model (LLM):
**Q&A_1:**
"
```md
{}
```
"
"""

LLM_generate_VFCS_2 = """
2. Q&A_2:
Here is the result of my second question using the large language model (LLM):
**Q&A_2:**

```md
{}
```
"""

staticAnalysis_report_abi = """
Here, using the slither static analysis tool, the static analysis report and ABI information for the current contract are displayed:

{}
"""

output_VFCS_format = """
[mainContractName]_id:
`function_1 -> function_2 -> function_3`(-> constructor)
"""

VFCS_example = """
Assuming the main contract's name is `mainContractName` (note that there may be multiple contracts in the same source code, but there can be only one main contract. Only the main contract's VFCS should be output, and the functions cannot be those modified as private!), the output format should be as follows:

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
The above is the complete information!
VFCS format:
{output_VFCS_format}

Here is a sample of the VFCS output format:
{VFCS_example}

There are a few points to note:
1. The `function_*` here cannot be functions that were determined to be private in Q&A_1 given by the LLM. Only functions present in the current contract should be output.
2. The `function_*` are considered important functions for constructing the VFCS.
3. The above is just an example, a sample that meets the VFCS format. The specific content to be output should be based on the specific contract source code and the given Q&A information and static analysis report!
4. The VFCS must be derived from the ABI! If a function is not given in the ABI, it cannot explicitly appear in the VFCS even if it is crucial!
5. You must carefully examine the content of the ABI. Many critical functions in the contract (such as those for transferring funds, calling other contracts, permission control, critical information judgment, etc.) may be marked as private in Solidity. However, private functions do not appear in the ABI! The VFCS we need cannot include statements modified by private! If this function needs to be shown, the closest parent function calling this function should be shown, and this parent function should not be modified by private! (Refer to the previous LLM Q&A)
6. Simply output the function name for function, without any other prefixes or suffixes. The function cannot output functions that are not present in the important functions given by Q&A_2 of the LLM!
7. You need to identify the main contract in the contract and replace the main contract name in the source code in the above format 'mainContractName_1' instead of directly outputting mainContractName_1.
Here is an example to help you understand what a main contract is:
**Main Contract** refers to the core contract in a multi-contract system that plays the primary role, managing and coordinating other contracts, and there can only be one main contract in a contract, not multiple main contracts.
```solidity
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
```
For example, in the contract above, there are two contracts, one is 'Governmental' and the other is 'Attacker', but because 'Governmental' plays the primary role, managing and coordinating other contracts, 'Governmental' is the main contract, and 'Attacker' is not the main contract. Therefore, the mainContractName_1 should be modified to Governmental_1 in the output.
8. The VFCS can involve multiple calls to the same function. It is not necessarily that a function can only appear once in a VFCS! For example, play()->play()->getProfit()->play()->getProfit() is also possible. This is a good example of both successive calls to the same function and interspersed calls to different functions! It is hoped that the length of the generated VFCS can not be infinite and do not output too many VFCS. The most important thing is to analyze whether this VFCS will indeed cause vulnerabilities.
9. Only output the VFCS of the main contract.
10. Only output 3 VFCS.

Please output all possible VFCS according to this format!
**Please pay attention!! You only need to output the final VFCS result without any instructions!!**
output:
"""
