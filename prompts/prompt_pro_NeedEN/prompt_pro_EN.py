

# introduction of VFCS

pro_1 = """
Next, here's the source code of the smart contract we need to analyze:
```sol
"""

# will show Source Code of SmartContract here



pro_2_totalBranchSnippet = """
```
Now, use the contract's VFCS as input to the Fuzzer for fuzzing!
For the same contract, it can conduct multiple VFCS and perform multiple fuzzing in serial, thus improving the branch coverage of fuzzing!

First, the Fuzzer will perform branch exploration on the current contract. And the branches that can be reached are all shown as follows (using code snippets from the source code):
**important**
```
{}
```

Here are some explanations:
1. For each of the above branches, there is a marking like "##(true branch)##" or "##(false branch)##". This actually means that in the fuzzing process, there is a test case that triggers this branch, and whether the branch enters or not, whether the branch condition is fulfilled or not, constitutes the two sides of the branch! Therefore, each branch has two possible situations.
2. For different branches, the code snippets given may be the same, because the code snippets shown only give one line of source code. If there are similar branching conditions in different functions of the main contract, it is possible to show the same code snippet.
3. Each branch is separated by "----------------".

"""


pro_3_sequence = """

For the above contract, such a sequence of VFCS has been fuzzing:
```
{}
```

For the sequence given above, there are a few things to note:
1. In the sequence, if a constructor appears, it corresponds to the empty position in the sequence.
2. Generally, VFCS will place the constructor at the end, so the last position in the sequence is usually empty.
3. Relationship with the contract source code: Only the functions in the main contract of the smart contract are included in the VFCS, so you don't need to pay attention to the auxiliary contracts in the source code. Therefore, the functions appearing in the sequence are all functions in the main contract.
4. Functions can be repeated multiple times or consecutively.

"""


pro_4_duration = """Using the VFCS sequence provided above as input, fuzzing in Fuzzer! After **{}** seconds,"""


# 需要传入时*100
pro_5_coverage = """"The current branch coverage is **{}%**!"""


pro_6_covered = """

After being tested by a Fuzzer, the current branch coverage rate and the code fragments that can be triggered are as follows:
```
{}
```

Here are a few notes:
1. It is possible to have duplicate covered branches:
    - The first case is that each branch uses the same other branch, because it may be "true" or "false".
    - The other case is that the conditions set by different functions are similar, in which case it is also possible to have the same code fragment, resulting in duplicate covered branches.
2. Each branch is separated by "==============".


"""


pro_7_target = """

Please refer to what I have given above:
1. Introduction to VFCS,
2, the specific contract source code,
3. All possible branch code snippets for the current contract,
4. The VFCS sequence currently used as input for fuzzing,
5. Branch coverage achieved after a period of time,
6. The specific code snippet that has been covered

** Requirements: **
Please combine the above six points to analyze: if fuzzing to achieve higher coverage, what will the next fuzzing VFCS look like?

** Format of the generated VFCS: **
mainContractName_1:
function1 -> function2 -> ...

mainContractName_2:
function1 -> function1 -> ...


** It's important to emphasize: **
-The generated VFCS are used as input for fuzzing, this fuzzing is based on the fuzzing of the last given VFCS. So the currently covered places will be given as known input to the next fuzzing round!
- Therefore, the focus should be on the branches that are not covered. New VFCS need to be generated to cover the currently not covered branches.

**Question:**
1. Summarize the current contract state
2. Analysis and Strategy
    - What uncovered branches are there? Identify them
    - Possible strategies:
        - **Repeated Calls**: Allow the function to appear multiple times in the sequence to increase the chance of covering untested branches
        - **Explore Different Paths**: Try different combinations that are more likely to have uncovered places
        - **Explore New Function Calls**: Since VFCS focuses on the call sequence between functions, consider adding new functions or different combinations of existing functions to the sequence to ensure coverage of all possible logical flows.
3. Design a new VFCS to cover the not covered branch (generate two possible VFCS sequences according to the given format)
"""
