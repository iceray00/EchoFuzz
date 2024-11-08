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

Instructions: After considering the above questions, please provide answers to the following questions and include both the questions and your responses (do not provide just the answers without the questions).

1. List the key functions in the source code (include all contracts and exclude any private functions; private functions must absolutely not be considered as key functions):
    - output: [key function name]
2. Specify the exact version of Solidity used in the source code (all versions are assumed to be 0.4.x):
    - output: [solidity version]    
3. List the private functions in the source code:
    - output: [private function]
4. Based on the mentioned version, what potential vulnerabilities could exist?
    - output: [possible vulnerabilities]
5. Based on the mentioned version, what types of vulnerabilities are not possible?
    - output: [impossible vulnerabilities]
6. After an initial analysis of the specific Solidity source code provided, what potential vulnerabilities might exist?
    - output: [possible vulnerabilities after analysis]
"""
