introduction_VFCS = """
Next, let’s discuss the field of smart contracts.

In the field of smart contracts, I propose a new paradigm. Please let me explain.
```
Introducing the "Vulnerable Function Call Sequence (VFCS)".

During the execution of smart contracts, transactions essentially manifest as continuous sequences of function calls. If a contract continues executing stably, these function call sequences can extend indefinitely. However, when cumulative changes in contract state cause a particular function call to crash or trigger an exception, that function call becomes the end-point of the VFCS. Meanwhile, the key function calls previously responsible for changing the state can be selectively included in the VFCS.

VFCS is not simply an ordered sequence of calls; rather, it represents a cumulative state change leading up to a particular function call that results in an exception or crash. In brief, VFCS is defined as follows:

"A series of function calls continuously alters the state of a contract; when the accumulated state changes reach a certain point, a specific function call results in an exception or crash—thus forming the VFCS."

To clarify:
* VFCS does not necessarily include all function calls; it only selects key function calls.
* If VFCS were to include all function calls, it would essentially represent a detailed transaction sequence, failing to effectively reduce the state space for vulnerability detection.
```

In practical scenarios, current techniques derived from Software Engineering—such as symbolic execution, static analysis, and rule-driven sequence generation—usually can only track explicit data dependencies among functions. They struggle to identify semantic relationships between function calls. For example, certain function calls might logically offset or neutralize one another's effects on state variables, forming a special semantic relationship --- known as offsetting interactions.

Thus, clearly, VFCS possesses the following three key properties:

1. Vulnerability Trigger: The final function call in a VFCS must execute under specific contract states capable of precisely triggering a smart contract vulnerability.

2. State Dependence Chain: Each function within the VFCS is crucial and modifies the contract’s state, fulfilling prerequisites for subsequent function calls in the sequence.

3. Minimality: A VFCS must meet a minimality condition—any proper subsequence of it must not trigger the same vulnerability. In other words, VFCS represents the minimal call sequence necessary to trigger a vulnerability. Removing any function call from the sequence renders the vulnerability non-triggerable.
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
