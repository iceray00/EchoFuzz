import os, sys

sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__))))

from import_VFCS import introduction_VFCS

"""
prompt_2_static_analysis: The second round starts with Static Analysis Feedback Analysis
"""

prompt_2_static_analysis_1 = f"""
{introduction_VFCS}

这里展示所需要检测的合约源代码:
"""

# will show: {source_code} in here


prompt_2_static_analysis_2 = """
对于上面的内容，这是我第一次使用大语言模型LLM提问时给出的结果：

**Q&A:**
"
```md
{}
```
"
**上面的内容可以提供参考！！但也仅仅参考！！**
"""

prompt_2_static_analysis_3 = """
为了让所生成的VFCS尽可能的准确，我使用静态分析工具Slither对上面给出的合约源代码进行分析，得出一些信息，可以进行参考，但不要输出违背注意事项的内容！

使用静态分析得出的内容包括：
1、执行命令`slither [solidity file name]`直接得出的静态分析后的整体检测报告(Analysis Report)。其中部分报告存在`Dangerous calls:`这样的关键词，请你注意！
2、执行`slither [file name] --print cfg`得出的控制流图(Control Flow Graph (CFG))
3、执行`slither [file name] --print call-graph`得出的函数调用图(Call Graph (CG))
4、执行`solc [file name] --abi`得出的应用二进制接口(Application Binary Interface (ABI))

**下面所给出的静态分析结果是你后面所要回答问题的重要分析依据！！**
**Static Analysis Result:**
```
{}
```
"""

prompt_2_static_analysis_target = """
请你根据上面的“标准”—— **静态分析报告内容** ，以及最关键的也是最一开始跟你提到的 **带状态VFCS** 的概念、以及“参考内容”—— **通过LLM得到的Q&A信息** ，回答一下以下内容：
（要求：请你输出时，包括问题也一起输出，不要只输出结果而不输出抛出的问题）

1、当前的这份source code中可能有哪些漏洞/漏洞类型？（之前LLM给出的Q&A，其中不可能的漏洞类型是值得参考的）
    - output: [possible vulnerability/vulnerability type]
2、你觉得现在的source code中比较重要的函数有哪些？————注意这里比较重要的函数应该是指主要会引发漏洞产生的函数，如果你判断出的比较重要的函数是被private修饰的，但因为在source code中被private修饰的函数（即私有方法）不应该作为重要函数而被你输出出来，所以你应该寻找离这个函数最近的，调用这个函数的父函数，将其父函数输出。只要是在source code中被private修饰的函数，都不算是比较重要的函数！
注意所有在之前LLM的Q&A中判断为被private修饰的函数都不要在这里输出出来！ 
    - output: [key function names]
3、你觉得现在在source code中哪一些关键的函数的组合调用会引起漏洞的产生呢？
    - output: [function call sequence]
4、在当前合约中，有哪些函数调用的组合，会改变变量的状态呢？
    - output: [function call sequence]
5、在source code中按照合约的不同进行划分，可能的VFCS会有哪些呢（注意VFCS里的函数不能是Q&A中被private修饰的函数（即私有函数））？
    - output: [possible VFCS]
6、在source code中，有可能存在多份合约，如果只有一份合约的话那么这个合约就是主合约，但如果存在多份合约，那请你判断哪个合约是主合约并输出主合约的名称
    - output: [main contract name]
"""



