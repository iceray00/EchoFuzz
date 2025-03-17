import os, sys

sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__))))

from import_VFCS import introduction_VFCS, solidity_version_diff

prompt_1_introduction = f"""
{introduction_VFCS}

现在，这里会展示出所需要检测的合约源代码:
"""

# will show: {source_code} in here


prompt_1_target = f"""
请你根据上面的合约源码（source code），思考以下的问题：
1、这个合约源码（一份源码中可能存在多个合约，你需要考虑全部的合约）中的关键函数是什么？——这里所说的关键函数是指：会影响合约状态的函数，或者间接、潜在的导致其他函数改变合约状态的函数。(注意在源码中被private修饰的函数（即私有方法）都不能作为关键函数！！)
2、当前这份source code中Solidity的具体版本是什么？
3、这份source code中不同的合约分别可能会出什么样的问题（漏洞）？
4、请留意当前合约的版本，你需要知道：{solidity_version_diff}


要求：当你思考完上述问题后，请你回答以下几个问题并输出你的回答（包括问题也一起输出，不要只输出结果而不输出提问的的问题）：

1、输出source code中的关键函数（包括全部合约，不要输出任何私有函数！私有函数一定不是关键函数！）：
    - output: [key function name]
2、输出source code中的函数之间从语意上的可能抵消关系：
    - output: [functions name]
3、输出当前source code中Solidity的具体版本（现版本均以0.4.x为准）：
    - output: [solidity version]    
4、输出当前source code中的私有函数，即被private修饰的函数：
    - output: [private function]
5、根据上述版本，有哪些可能出现的漏洞呢？
    - output: [possible vulnerabilities]
6、根据上述版本，不可能出现的漏洞类型有哪些呢？
    - output: [impossible vulnerabilities]
7、根据具体给出的智能合约solidity具体源代码，经过初步分析，可能有哪些漏洞呢？
    - output: [possible vulnerabilities after analysis]
"""