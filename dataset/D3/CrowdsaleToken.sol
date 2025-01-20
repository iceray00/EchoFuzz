/**
 *Submitted for verification at Etherscan.io on 2024-08-21
*/

pragma solidity ^0.4.23;

library SafeMath {
    function mul(uint256 a, uint256 b) internal pure returns (uint256) {
        if (a == 0) {
            return 0;
        }
        uint256 c = a * b;
        assert(c / a == b);
        return c;
    }

    function sub(uint256 a, uint256 b) internal pure returns (uint256) {
        assert(b <= a);
        return a - b;
    }

    function add(uint256 a, uint256 b) internal pure returns (uint256 c) {
        c = a + b;
        assert(c >= a);
        return c;
    }

    function div(uint256 a, uint256 b) internal pure returns (uint256) {
        // Solidity automatically throws when dividing by 0
        assert(b > 0);
        uint256 c = a / b;
        assert(a == b * c + a % b); // This always holds
        return c;
    }
}

// CargoCoin and other contracts follow, utilizing SafeMath as needed


/**
 * @title Ownable
 * @dev The Ownable contract has an owner address, and provides basic authorization control functions,
 * this simplifies the implementation of "user permissions".
 **/
contract Ownable {
    address public owner;
    event OwnershipTransferred(address indexed previousOwner, address indexed newOwner);

    constructor() public {
        owner = msg.sender;
    }

    modifier onlyOwner() {
        require(msg.sender == owner);
        _;
    }

    function transferOwnership(address newOwner) public onlyOwner {
        require(newOwner != address(0));
        emit OwnershipTransferred(owner, newOwner);
        owner = newOwner;
    }
}

/**
 * @title ERC20Basic interface
 * @dev Basic ERC20 interface
 **/
contract ERC20Basic {
    function totalSupply() public view returns (uint256);
    function balanceOf(address who) public view returns (uint256);
    function transfer(address to, uint256 value) public returns (bool);
    event Transfer(address indexed from, address indexed to, uint256 value);
}

/**
 * @title ERC20 interface
 * @dev see https://github.com/ethereum/EIPs/issues/20
 **/
contract ERC20 is ERC20Basic {
    function allowance(address owner, address spender) public view returns (uint256);
    function transferFrom(address from, address to, uint256 value) public returns (bool);
    function approve(address spender, uint256 value) public returns (bool);
    event Approval(address indexed owner, address indexed spender, uint256 value);
}

/**
 * @title Basic token
 * @dev Basic version of StandardToken, with no allowances.
 **/
contract BasicToken is ERC20Basic {
    using SafeMath for uint256;
    mapping(address => uint256) balances;
    uint256 totalSupply_;
    
    function totalSupply() public view returns (uint256) {
        return totalSupply_;
    }
    
    function transfer(address _to, uint256 _value) public returns (bool) {
        require(_to != address(0));
        require(_value <= balances[msg.sender]);
        
        balances[msg.sender] = balances[msg.sender].sub(_value);
        balances[_to] = balances[_to].add(_value);
        emit Transfer(msg.sender, _to, _value);
        return true;
    }
    
    function balanceOf(address _owner) public view returns (uint256) {
        return balances[_owner];
    }
}

contract StandardToken is ERC20, BasicToken, Ownable {
    mapping (address => mapping (address => uint256)) internal allowed;
    
    function transferFrom(address _from, address _to, uint256 _value) public returns (bool) {
        require(_to != address(0));
        require(_value <= balances[_from]);
        require(_value <= allowed[_from][msg.sender]);
    
        balances[_from] = balances[_from].sub(_value);
        balances[_to] = balances[_to].add(_value);
        allowed[_from][msg.sender] = allowed[_from][msg.sender].sub(_value);
        
        emit Transfer(_from, _to, _value);
        return true;
    }
    
    function approve(address _spender, uint256 _value) public returns (bool) {
        allowed[msg.sender][_spender] = _value;
        emit Approval(msg.sender, _spender, _value);
        return true;
    }
    
    function allowance(address _owner, address _spender) public view returns (uint256) {
        return allowed[_owner][_spender];
    }
    
    function increaseApproval(address _spender, uint _addedValue) public returns (bool) {
        allowed[msg.sender][_spender] = allowed[msg.sender][_spender].add(_addedValue);
        emit Approval(msg.sender, _spender, allowed[msg.sender][_spender]);
        return true;
    }
    
    function decreaseApproval(address _spender, uint _subtractedValue) public returns (bool) {
        uint oldValue = allowed[msg.sender][_spender];
        if (_subtractedValue > oldValue) {
            allowed[msg.sender][_spender] = 0;
        } else {
            allowed[msg.sender][_spender] = oldValue.sub(_subtractedValue);
        }
        emit Approval(msg.sender, _spender, allowed[msg.sender][_spender]);
        return true;
    }
}

/**
 * @title Configurable
 * @dev Configurable variables of the contract
 **/
contract Configurable is StandardToken {
    uint256 constant cap = 20000000000 * 10**18; // Updated cap
    uint256 basePrice = 5000 * 10**18; // tokens per 1 ether
    uint256 public tokensSold = 0;
    uint public tokensToSell = 0;
    uint256 public constant tokenReserve = 10000000000 * 10**18; // Updated reserve
    uint256 public remainingTokens = 0;
}

/**
 * @title CrowdsaleToken 
 * @dev Contract to perform crowd sale with token
 **/
contract CrowdsaleToken is Configurable {
    uint256 public basePriceRound1 = basePrice;
    uint256 public basePriceRound2 = 4000 * 10**18;
    uint256 public basePriceRound3 = 3500 * 10**18;
    
    enum Stages {
        none,
        round1, 
        round2,
        round3,
        icoEnd
    }
    
    Stages currentStage;
    
    modifier checkValidation {
        require(currentStage != Stages.none && currentStage != Stages.icoEnd);
        require(msg.value >= 0.5 ether);
        require(remainingTokens > 0);
        _;
    }
  
    constructor() public {
        currentStage = Stages.none;
        balances[owner] = balances[owner].add(tokenReserve);
        totalSupply_ = totalSupply_.add(cap);
        tokensToSell = totalSupply_.sub(tokenReserve);
        remainingTokens = totalSupply_.sub(tokenReserve);
        emit Transfer(address(this), owner, tokenReserve);
    }
    
    function () public payable checkValidation {
        uint256 weiAmount = msg.value;
        uint256 tokens = weiAmount.mul(basePrice).div(1 ether);
        uint256 returnWei = 0;
        
        if (tokensSold.add(tokens) > cap) {
            uint256 newTokens = cap.sub(tokensSold);
            uint256 newWei = newTokens.div(basePrice).mul(1 ether);
            returnWei = weiAmount.sub(newWei);
            weiAmount = newWei;
            tokens = newTokens;
        }
        
        tokensSold = tokensSold.add(tokens);
        remainingTokens = tokensToSell.sub(tokensSold);
        if (returnWei > 0) {
            msg.sender.transfer(returnWei);
            emit Transfer(address(this), msg.sender, returnWei);
        }
        
        balances[msg.sender] = balances[msg.sender].add(tokens);
        emit Transfer(address(this), msg.sender, tokens);
        
        owner.transfer(weiAmount);
    }
    
    function startRound1() public onlyOwner {
        require(currentStage == Stages.none);
        currentStage = Stages.round1;
    }
    
    function startRound2() public onlyOwner {
        require(currentStage == Stages.round1);
        currentStage = Stages.round2;
        basePrice = 4000 * 10**18;
    }
    
    function startRound3() public onlyOwner {
        require(currentStage == Stages.round2);
        currentStage = Stages.round3;
        basePrice = 3500 * 10**18;
    }

    function endIco() internal {
        currentStage = Stages.icoEnd;
        if (remainingTokens > 0) {
            balances[owner] = balances[owner].add(remainingTokens);
        }
    }
    
    function finalizeIco() public onlyOwner {
        require(currentStage == Stages.round3);
        endIco();
    }
}

/**
 * @title CargoCoin 
 * @dev Contract to create the CargoCoin Token
 **/
contract CargoCoin is CrowdsaleToken {
    string public constant name = "CargoCoin";
    string public constant symbol = "CgC";
    uint32 public constant decimals = 18;
}