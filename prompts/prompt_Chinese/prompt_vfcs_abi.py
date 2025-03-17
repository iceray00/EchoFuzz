VFCS_abi_1 = """
**现在，你的任务就是帮我根据VFCS信息与原本的abi，转化成相应的abi的信息！**
* 根据VFCS、原本的abi信息，来生成符合VFCS的abi信息！
* 也就是实现：abi、VFCS -> vfcs_abi 


vfcs_abi的要求：(在原有的abi基础上，根据VFCS给出的信息，进行修改以得到vfcs_abi)
1、新增"order"字段，来指示当前的这个函数，会在哪被调用。也就是，当前函数的"order"值就是在最终完整的VFCS中所在的位置。
  - 如果当前函数在VFCS中只出现了一次，那么order_type就是"int"，然后在"order"字段中就给出所属的位置；
  - 如果当前函数在VFCS中，被反复调用操控，那么在"order"中就如:`\"order\": [0, 2]`这样出现！这代表当前函数是在VFCS中第一个、和第三个调用的函数！
2、在ABI中的constructor，也就是合约中的构造函数，在这里规定：必须在VFCS的最后一个位置！这是一个特例！因为在正式执行中，构造函数会在额外的第一次时就被调用！所以我们给出显式的VFCS，仅仅只需要把构造函数的排序放在最后一个就可以了！
 
**示例**
question:
**input abi information:**
"[{\"constant\":false,\"inputs\":[],\"name\":\"getProfit\",\"outputs\":[],\"payable\":false,\"type\":\"function\"},{\"constant\":false,\"inputs\":[{\"name\":\"number\",\"type\":\"uint256\"}],\"name\":\"play\",\"outputs\":[],\"payable\":true,\"type\":\"function\"},{\"constant\":true,\"inputs\":[{\"name\":\"\",\"type\":\"uint256\"}],\"name\":\"players\",\"outputs\":[{\"name\":\"addr\",\"type\":\"address\"},{\"name\":\"number\",\"type\":\"uint256\"}],\"payable\":false,\"type\":\"function\"},{\"inputs\":[],\"type\":\"constructor\"}]\n"

**input VFCS information:**
OddsAndEvens_1:
constructor->play->getProfit->play->getProfit

OddsAndEvens_2:
constructor->play->play->getProfit->getProfit
#other introductions


answer:
```json
OddsAndEvens_1:
"[{\"constant\": false,\"inputs\": [],\"name\":\"getProfit\",\"outputs\": [],\"payable\": false,\"stateMutability\":\"nonpayable\",\"type\":\"function\",\"order\": [1, 3]}, {\"constant\": false,\"inputs\": [{\"name\":\"number\",\"type\":\"uint256\"}],\"name\":\"play\",\"outputs\": [],\"payable\": true,\"stateMutability\":\"payable\",\"type\":\"function\",\"order\": [0, 2]}, {\"constant\": true,\"inputs\": [{\"name\":\"\",\"type\":\"uint256\"}],\"name\":\"players\",\"outputs\": [{\"name\":\"addr\",\"type\":\"address\"}, {\"name\":\"number\",\"type\":\"uint256\"}],\"payable\": false,\"stateMutability\":\"view\",\"type\":\"function\"}, {\"inputs\": [],\"payable\": false,\"stateMutability\":\"nonpayable\",\"type\":\"constructor\",\"order\": [4]}]"

OddsAndEvens_2:
"[{\"constant\": false,\"inputs\": [],\"name\":\"getProfit\",\"outputs\": [],\"payable\": false,\"stateMutability\":\"nonpayable\",\"type\":\"function\",\"order\": [2, 3]}, {\"constant\": false,\"inputs\": [{\"name\":\"number\",\"type\":\"uint256\"}],\"name\":\"play\",\"outputs\": [],\"payable\": true,\"stateMutability\":\"payable\",\"type\":\"function\",\"order\": [0, 1]}, {\"constant\": true,\"inputs\": [{\"name\":\"\",\"type\":\"uint256\"}],\"name\":\"players\",\"outputs\": [{\"name\":\"addr\",\"type\":\"address\"}, {\"name\":\"number\",\"type\":\"uint256\"}],\"payable\": false,\"stateMutability\":\"view\",\"type\":\"function\"}, {\"inputs\": [],\"payable\": false,\"stateMutability\":\"nonpayable\",\"type\":\"constructor\",\"order\": [4]}]"
```



最后，有几条需要注意的地方！
第一点：由于构造函数需要放在VFCS的最后一个！所以如果VFCS中存在中途有构造函数的情况，都需要将构造函数放在最后的位置，其他函数依次从开始到构造函数前。比如VFCS的信息为constructor->play->getProfit->play->getProfit，那么正确的顺序应该是play->getProfit->play->getProfit->constructor，因此输出的abi内容就需要将constructor的信息放在最后！
又比如：VFCS的信息为play->getProfit->constructor->play->getProfit，因为构造函数constructor必须放在最后，且其他的函数按照原来的排列方式依次接上，那么正确的顺序应该是play->getProfit->play->getProfit->constructor，因此输出的abi内容也应该是按照正确的顺序对应输出abi，abi内容为：
"[{\"constant\": false,\"inputs\": [],\"name\":\"getProfit\",\"outputs\": [],\"payable\": false,\"stateMutability\":\"nonpayable\",\"type\":\"function\",\"order\": [1, 3]}, {\"constant\": false,\"inputs\": [{\"name\":\"number\",\"type\":\"uint256\"}],\"name\":\"play\",\"outputs\": [],\"payable\": true,\"stateMutability\":\"payable\",\"type\":\"function\",\"order\": [0, 2]}, {\"constant\": true,\"inputs\": [{\"name\":\"\",\"type\":\"uint256\"}],\"name\":\"players\",\"outputs\": [{\"name\":\"addr\",\"type\":\"address\"}, {\"name\":\"number\",\"type\":\"uint256\"}],\"payable\": false,\"stateMutability\":\"view\",\"type\":\"function\"}, {\"inputs\": [],\"payable\": false,\"stateMutability\":\"nonpayable\",\"type\":\"constructor\",\"order\": [4]}]"
第二点：abi的内容中包含的函数名称和调用顺序，应该是VFCS中每条信息里有的函数，并且order后的顺序应该与VFCS的顺序一致!
第三点：最后输出时，只需要输出abi的内容就可以了！其他都不用输出，```也不用输出！
第四点：输出完全部的abi内容之后就不要再输出任何其他的东西了。
第五点：在生成的vfcs_abi内容中，对于"order"字段冒号后面的内容，也就是[]里面的内容必须是在当前abi中每个order里的数字都要能够连接起来。比如：如果vfcs为constructor->play->getProfit->play->getProfit，那么生成的abi应该是下面这样的：
"[{\"constant\": false,\"inputs\": [],\"name\":\"getProfit\",\"outputs\": [],\"payable\": false,\"stateMutability\":\"nonpayable\",\"type\":\"function\",\"order\": [1, 3]}, {\"constant\": false,\"inputs\": [{\"name\":\"number\",\"type\":\"uint256\"}],\"name\":\"play\",\"outputs\": [],\"payable\": true,\"stateMutability\":\"payable\",\"type\":\"function\",\"order\": [0, 2]}, {\"constant\": true,\"inputs\": [{\"name\":\"\",\"type\":\"uint256\"}],\"name\":\"players\",\"outputs\": [{\"name\":\"addr\",\"type\":\"address\"}, {\"name\":\"number\",\"type\":\"uint256\"}],\"payable\": false,\"stateMutability\":\"view\",\"type\":\"function\"}, {\"inputs\": [],\"payable\": false,\"stateMutability\":\"nonpayable\",\"type\":\"constructor\",\"order\": [4]}]"
这段abi是根据VFCS生成的，并且其中的每个order字段后数字内的内容合起来是完整连贯的，也就是 [1, 3]，[0, 2]，[4]，合起来是完整的从0到4，表示了VFCS中共有五个函数，构造函数constructor默认在最后面！
第六点：输出最后的vfcs_abi时，记得加上引号！正确的："[{...}, {...}, ...]"，错误的：[{...}, {...}, ...]！
第七点：输出abi时，不要换行！直接跟上面给出的例子类似的格式就可以了！
例如：正确的："[{\"constant\":........}]"，错误的是：
"[
{\"constant\": ........}
]"
这样在两端有换行是非常错误的！不要这样处理！
第八点：生成的时候，作为标题的、例如上面例子给出的 OddsAndEvens_1: 直接给出即可！也就是contractName_id:的格式就可以了！不要加上引号！也不要加上``来修饰！直接OddsAndEvens_1:就可以了！

请你按照这个格式和上述注意事项输出所有可能的VFCS！
**Please pay attention!! You only need to output the final VFCS result without any instructions!!**


question:
"""

input_abi = """
**input abi information:**
{}
"""

input_VFCS = """
**input VFCS information:**
{}
"""

VFCS_abi_2 = """
answer:
"""

