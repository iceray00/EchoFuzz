import os, sys

sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__))))

from import_VFCS import introduction_VFCS, solidity_version_diff

prompt_1_introduction = f"""
{introduction_VFCS}

Now, here is the source code of the contract to be checked:
"""

# will show: {source_code} in here

prompt_1_target = f"""Please review the provided smart contract source code and address the following questions:

1. What are the key functions in this source code? (Consider all contracts present in the source code). Key functions are those that can change the state of the contract directly or indirectly. Note that functions marked as `private` should not be considered as key functions.
2. What is the specific version of Solidity used in this source code?
3. What potential issues (vulnerabilities) might arise in the different contracts within this source code?
4. Pay attention to the current contract version and take note of the Solidity version differences: {solidity_version_diff}

Instructions: After considering the above discussion, please answer the following questions and clearly output your answers. (Include both the questions and the answers together; do not output the answers alone.)

1. Output the key functions in the source code (including all contracts, excluding any private functions; private functions are definitely not key functions!):
    - output: [key function name]
2. Output functions in the source code that may have semantic offsetting relationships:
    - output: [functions name]    
3. Specify the exact version of Solidity used in the source code (all versions are assumed to be 0.4.x):
    - output: [solidity version]    
4. List the private functions in the source code:
    - output: [private function]
5. Based on the mentioned version, what potential vulnerabilities could exist?
    - output: [possible vulnerabilities]
6. Based on the mentioned version, what types of vulnerabilities are not possible?
    - output: [impossible vulnerabilities]
7. After an initial analysis of the specific Solidity source code provided, what potential vulnerabilities might exist?
    - output: [possible vulnerabilities after analysis]
"""
