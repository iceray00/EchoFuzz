
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
6. You need to identify the main contract in the contract and replace the main contract name in the source code in the above format 'mainContractName_1' instead of directly outputting mainContractName_1.
Here is an example to help you understand what a main contract is:
**Main Contract** refers to the core contract in a multi-contract system that plays the primary role, managing and coordinating other contracts, and there can only be one main contract in a contract, not multiple main contracts.
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
For example, in the above contract scenario, there are two contracts: one named 'Crowdsale' and the other named 'Attacker'. However, since the 'Crowdsale' contract plays the main role in managing and coordinating other contracts, 'Crowdsale' is considered the main contract, whereas 'Attacker' is not. Therefore, whenever outputting results, mainContractName_1 must be updated to Crowdsale_1.
Please take special notice of the following points:
1. The most probable VFCS is: invest -> invest -> withdraw. Explanation:
    a. The functions invest and refund have an offsetting relationship. If called sequentially, the refund function resets the variable invested, thereby offsetting previous calls to invest.
    b. To reach the state phase = 1, the invest function must be called consecutively at least twice.
    c. Vulnerability trigger: The vulnerability associated with the withdraw function requires the condition phase = 1. Thus, the sequence becomes:
        invest -> invest -> withdraw

2. You'll notice: VFCS can involve repeated calls to the same function multiple times; the function does not have to appear only once. Thus, VFCS sequences may include repeated calls of the same function.

3. Only output the VFCS of the main contract.

4. Please output the 3 most probable VFCS.


Please output all possible VFCS according to this format!
**Please pay attention!! You only need to output the final VFCS result without any instructions!!**
output:

"""
