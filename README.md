# EchoFuzz: Empowering Smart Contract Fuzzing with Large Language Models

If you want to use a locally deployed LLM to run our framework, see `LLM/README.md` for details.

For information about our tool's dependencies, how to install them, and how to install, initialize and use our framework, you can see `fuzzer/README.md`


# Inital


Our framework is implemented based on previous work [IR-Fuzz](https://github.com/Messi-Q/IR-Fuzz), so we will install it first.

```bash
git clone https://github.com/Messi-Q/IR-Fuzz.git
```


then, please install the dependencies:
```bash
sudo apt update
sudo apt install -y cmake golang-go libleveldb-dev libcrypto++-dev software-properties-common
sudo add-apt-repository -y ppa:ethereum/ethereum
```

```bash
sudo apt update
sudo apt install -y build-essential git libssl-dev libgmp-dev libboost-all-dev ethereum libjsoncpp-dev
pip3 install -r requirements.txt
```

Next, we install the _Solidity_ compiler `solc`. If it is already installed, you can ignore it. We use the integrated tool [solc-select](https://github.com/crytic/solc-select) to achieve this.

```bash
pip3 install numpy solc-select
solc-select use 0.4.26 --always-install
```

Sometimes, the default **Go** module proxy server may experience network issues. In this case, you may need to switch to a more stable proxy, such as GOPROXY. Set the GOPROXY environment variable to https://goproxy.io or https://proxy.golang.org

or you can exec:
```bash
export GOPROXY=https://goproxy.io
```

### Check it works:

```bash
cmake --version
go version
geth version
solc --version
```

### Check `go` in bran works:
```bash
cd bran
go mod init
cd ..
```

## initial EchoFuzz


you can initial **EchoFuzz** by following script: (in dir `fuzzer`)

```bash
sh initial_EchoFuzz.sh
```


# Usage

You can run EchoFuzz by executable file `run_echofuzz.sh`

```bash
./run_echofuzz.sh $DURATION $ROUND $MODEL_NAME
```


* Use LLM: (We have integrated [DeepBrick's](https://deepbricks.ai) API call method.)
```bash
export DB_API_KEY=[your_deepbricks_api_key]
```

* If you want to use EchoFuzz in local system with `ollama`, you can see `LLM/README.md`.


