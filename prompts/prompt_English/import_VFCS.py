introduction_VFCS = """
Next, let's discuss the field of smart contracts.

In the realm of smart contracts, I want to propose a new paradigm. Please allow me to explain. You may refer to this paper: "(Usenix Security'21) SMARTEST Effectively Hunting Vulnerable Transaction Sequences in Smart Contracts through Language Model-Guided Symbolic Execution." If you can't find it, don't worry; I'll give you a brief overview.

In this paper (SmarTest), the concept of "Vulnerable Transaction Sequence (VTS)" is introduced. It is defined as a continuously inputted transaction sequence that crashes or behaves abnormally at a certain transaction. This specific transaction is considered the last transaction of the VTS, while the preceding transactions are those that alter the contract state. Such a sequence of transactions, capable of changing the contract state and ultimately leading to a crash, is termed VTS.

Please note that this VTS is stateful. Inspired by VTS, I propose the following concept: VFCS.

Proposing the "Vulnerable Function Call Sequence (VFCS)."

Background: From the perspective of smart contracts, continuously inputting transactions forms a sequence of consecutive transactions. From the perspective of smart contract source code, this translates to continuous function calls—whether it’s a single transaction or multiple transactions, internally, it manifests as a continuous sequence of function calls. If the contract executes without crashing, this sequence can be considered infinitely long. This is the foundation for proposing VFCS, transforming consecutive transactions into a continuous function call relationship.

The "function call sequence" mentioned here is not about functions calling each other but rather the contract calling functions during transaction execution. The proposed "Vulnerable Function Call Sequence (VFCS)" refers to: at the function level in the contract source code, whether single or multiple transactions continuously call functions. If a crash or anomaly occurs, it reflects at the function level and could only happen at a specific function or among several functions simultaneously, with the last crashing function call marking the end of the VFCS. Previous function calls that altered the contract state are selectively included in the VFCS.

Similarly, this VFCS, like the VTS in SmarTest, is stateful. It is not merely a sequence of function calls but rather a stateful function call that, due to certain values and preceding state-altering function calls, leads to a crash or anomaly.

A High-Level Definition of VFCS: Continuous function calls that progressively change the contract's state, culminating in a crash/anomalous behavior at a particular function at some point, mark the end of the VFCS.
(This indeed resembles SmarTest's definition of VTS, but it is more granular, focusing on the function level and considering state space compression.)

Intuitively, if such a VFCS is triggered during continuous transactions, it indicates that the current transaction sequence is vulnerable.

It is crucial to emphasize: the function call sequence here does not necessarily include all functions involved in the calls.

One important point: (in an extreme case) if all function calls are included in this "Vulnerable Function Call Sequence (VFCS)," it essentially becomes a variant of the specific transaction sequence—just a more detailed representation of the transaction sequence, without reducing the state space for target vulnerability hunting. Conversely, in this manner, the snapshot templates required for fuzzing will be fewer, needing only parameter values.

However, if the function sequence in VFCS consists only of critical functions with vulnerable states, this significantly reduces the state space for target hunting. Correspondingly, defining the snapshot templates becomes more complex, with more aspects to consider during fuzzing.

This presents a "Trade-Off." Ultimately, the design's accuracy and efficiency of the tool are influenced by these two aspects: extracting VFCS from the contract and conducting snapshot fuzzing on VFCS to identify vulnerabilities triggering crashes.
"""


# VFCS_format = """
# ```md
# VFCS:
# 1. Initialization Phase
#     - Identify the key functions of the contract.
#     - Define initial state variables and their values.
#     - Identify and call initialization functions.
#
# 2. Function Call Sequence
#     - For each function call:
#         a. Define input parameters.
#         b. Call the function.
#         c. Capture the state before and after the call.
#         d. Check for state changes.
#
# 3. State Analysis
#     - Analyze the state changes after each function call.
#     - Identify potential vulnerabilities based on state inconsistencies or unexpected changes.
#
# 4. Termination Condition
#     - Define the termination condition for the sequence (e.g., a function call leads to a crash/exception).
#
# 5. Vulnerability Function Call Sequence with State
#     - Possible VFCS with state
# ```
# """

## Potential Vulnerabilities in Current Version:
# solidity_version_diff = """

# - Integer overflow and underflow
# - Reentrancy attacks
# - Delegatecall vulnerabilities
# - Forced sending attacks
# - Unchecked send and transfer return values
# - Improper handling of low-level call failures
# - Missing function visibility
# - Uninitialized storage pointers
# - Logic and syntax errors
# - Insecure random number generation
# - Block attribute dependencies
# - Unsafe external calls

# ## Vulnerabilities Not Present in Current Version:
# To comprehensively cover the types of vulnerabilities that do not appear in various versions, here is a list of specific vulnerabilities that have been improved or eliminated in each version:

# ### Solidity 0.4.x Versions

# **Vulnerabilities Not Present:**
# - Unspecified visibility: After this version, all functions must have specified visibility.
# - Non-strict type conversions: After version 0.4.x, type conversions became more stringent.
# """


solidity_version_diff = """
##Potential Vulnerabilities in Each Version:

**Solidity 0.4.x Versions**

- Integer overflow and underflow
- Reentrancy attacks
- Delegatecall vulnerabilities
- Forced sending attacks
- Unchecked send and transfer return values
- Improper handling of low-level call failures
- Missing function visibility
- Uninitialized storage pointers
- Logic and syntax errors
- Insecure random number generation
- Block attribute dependencies
- Unsafe external calls

"""



# solidity_version_diff_other = """
##Potential Vulnerabilities in Each Version:
# **Solidity 0.5.x Versions**

# - Integer overflow and underflow
# - Reentrancy attacks
# - Delegatecall vulnerabilities
# - Forced sending attacks
# - Unchecked send and transfer return values
# - Improper handling of low-level call failures
# - Missing function visibility
# - Uninitialized storage pointers
# - Logic and syntax errors
# - Insecure random number generation
# - Block attribute dependencies
# - Unsafe external calls
# - String handling errors
# - Event handling errors

# **Solidity 0.6.x Versions**

# - Integer overflow and underflow
# - Reentrancy attacks
# - Delegatecall vulnerabilities
# - Forced sending attacks
# - Unchecked send and transfer return values
# - Improper handling of low-level call failures
# - Missing function visibility
# - Uninitialized storage pointers
# - Logic and syntax errors
# - Insecure random number generation
# - Block attribute dependencies
# - Unsafe external calls
# - String handling errors
# - Event handling errors
# - Assertion failures
# - Specific compiler optimization errors

# **Solidity 0.7.x Versions**

# - Integer overflow and underflow
# - Reentrancy attacks
# - Delegatecall vulnerabilities
# - Forced sending attacks
# - Unchecked send and transfer return values
# - Improper handling of low-level call failures
# - Missing function visibility
# - Uninitialized storage pointers
# - Logic and syntax errors
# - Insecure random number generation
# - Block attribute dependencies
# - Unsafe external calls
# - String handling errors
# - Event handling errors
# - Assertion failures
# - Specific compiler optimization errors
# - Constructor errors
# - Function selector conflicts

# **Solidity 0.8.x Versions**

# - Reentrancy attacks
# - Delegatecall vulnerabilities
# - Forced sending attacks
# - Unchecked send and transfer return values
# - Improper handling of low-level call failures
# - Missing function visibility
# - Uninitialized storage pointers
# - Logic and syntax errors
# - Insecure random number generation
# - Block attribute dependencies
# - Unsafe external calls
# - String handling errors
# - Event handling errors
# - Assertion failures
# - Specific compiler optimization errors
# - Constructor errors
# - Function selector conflicts

# ## Vulnerabilities Not Present in Each Version:

# To comprehensively cover the types of vulnerabilities that do not appear in various versions, here is a list of specific vulnerabilities that have been improved or eliminated in each version:

# ### Solidity 0.4.x Versions

# **Vulnerabilities Not Present:**
# - Unspecified visibility: After this version, all functions must have specified visibility.
# - Non-strict type conversions: After version 0.4.x, type conversions became more stringent.

# ### Solidity 0.5.x Versions

# **Vulnerabilities Not Present:**
# - Undeclared function visibility: This version mandates that all functions declare visibility.
# - Implicit string and byte array conversions: After version 0.5.x, implicit conversions between strings and byte arrays were prohibited.
# - Non-strict abi.encode and abi.encodePacked: In this version, encoding became stricter, reducing potential encoding errors.

# ### Solidity 0.6.x Versions

# **Vulnerabilities Not Present:**
# - Undeclared receive function: Starting from version 0.6.x, the receive function is used for pure Ether transfers.
# - Implicit fallback function: The fallback function must be explicitly declared.
# - Non-strict event signatures: Event signatures became stricter, reducing event handling errors.
# - Unsafe global variable usage: After this version, the usage of global variables became stricter and safer.

# ### Solidity 0.7.x Versions

# **Vulnerabilities Not Present:**
# - Implicit constructor: This version requires all constructors to be explicitly declared.
# - Non-explicit storage pointer assignment: In version 0.7.x, storage pointer assignment became stricter.
# - Uninitialized state variables: After this version, the compiler strictly checks for uninitialized state variables.
# - Non-explicit pragma version declaration: Mandates explicit pragma version declaration, ensuring compiler version consistency.

# ### Solidity 0.8.x Versions

# **Vulnerabilities Not Present:**
# - Integer overflow and underflow: Version 0.8.x enabled integer overflow and underflow checks by default, preventing such vulnerabilities.
# - Implicit usage of unchecked: Integer operations default to overflow checks; unchecked must be explicitly used to disable overflow checks.
# - Non-strict type conversions: Type conversions are strictly checked, preventing implicit conversion errors.
# - Undeclared receive function: Explicit declaration of the receive function is required for Ether transfers.
# - Undeclared fallback function: Explicit declaration of the fallback function is required.
# - Non-strict event signatures: Event signatures are strictly checked, reducing event handling errors.
# - Unsafe global variable usage: Global variable usage is strictly checked, ensuring safety.
# - Implicit constructor: All constructors must be explicitly declared, preventing implicit behavior.
# - Uninitialized state variables: The compiler strictly checks for uninitialized state variables, ensuring state consistency.
# - Non-explicit pragma version declaration: Explicit pragma version declaration is mandatory, ensuring compiler version consistency.
# """
