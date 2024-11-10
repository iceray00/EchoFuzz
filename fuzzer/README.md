
# install _EchoFuzz_

## Dependencies

#### Dependency Configuration:

* CMake: >=3.5.1
* Python: >=3.5（ideally 3.6）
* Go: >=1.15
* leveldb 1.20
* Geth & Tools: geth, evm, etc
* solc: 0.4.26
* numpy

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
```

Next, we install the _Solidity_ compiler `solc`. If it is already installed, you can ignore it.  We use the integrated tool [solc-select](https://github.com/crytic/solc-select) to achieve this.

```bash
pip3 install numpy solc-select
solc-select use 0.4.26 --always-install
```

Sometimes, the default **Go** module proxy server may experience network issues. In this case, you may need to switch to a more stable proxy, such as GOPROXY. Set the GOPROXY environment variable to https://goproxy.io or https://proxy.golang.org

or you can exec:
```bash
export GOPROXY=https://goproxy.io
```

### Check working:

```bash
cmake --version
go version
geth version
solc --version
```

### Check `go` in bran working:
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


## Run

You can run EchoFuzz by executable file `run_echofuzz.sh`

```bash
./run_echofuzz.sh $Duration $Round $ModelName
```





