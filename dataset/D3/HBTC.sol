pragma solidity ^0.4.25;

contract ERC20Interface {
    function totalSupply() public constant returns (uint);
    function balanceOf(address tokenOwner) public constant returns (uint balance);
    function allowance(address tokenOwner, address spender) public constant returns (uint remaining);
    function transfer(address to, uint tokens) public returns (bool success);
    function approve(address spender, uint tokens) public returns (bool success);
    function transferFrom(address from, address to, uint tokens) public returns (bool success);

    event Transfer(address indexed from, address indexed to, uint _value);
    event Approval(address indexed tokenOwner, address indexed spender, uint _value);
    event Burn(address indexed from, address indexed to, uint _value);
    event Mint(address indexed from, address indexed to, uint _value);
}

contract TESNET {
    address public deployer;
	
    constructor() public {
        deployer = msg.sender;
    }

	modifier restricted {
        require(msg.sender == deployer);
        _;
    }
	
}

contract HBTC is ERC20Interface, TESNET {
	
    string 	public symbol;
    string 	public name;
    uint8 	public decimals;
    uint256 private _totalSupply;

    uint256 public mintstatus;

    mapping(address => uint256) balances;
    mapping(address => mapping(address => uint)) allowed;
	
	/*==============================
    =          CONSTRUCTOR         =
    ==============================*/  

    // Testnet Token
	
    constructor() public {
        symbol          = "HBTC";
        name            = "HBTC";
        decimals        = 8;
        _totalSupply    = 100000000;	// Max Supply 21,000,000
        mintstatus      = 1;
		
        balances[msg.sender] = 100000000; // 1 HBTC
        emit Transfer(address(0), msg.sender, 100000000); // 1 HBTC
    }

    function transfer(address to, uint256 _value) public returns (bool success) {
		if (to == 0x0) revert();                               
		if (_value <= 0) revert(); 
        if (balances[msg.sender] < _value) revert();           		
        if (balances[to] + _value < balances[to]) revert(); 		
		
        balances[msg.sender] 		= sub(balances[msg.sender], _value);
        balances[to] 				= add(balances[to], _value);
        emit Transfer(msg.sender, to, _value);
        return true;
    }
	
    function approve(address spender, uint256 _value) public returns (bool success) {
		if (_value <= 0) revert(); 
        allowed[msg.sender][spender] = _value;
        emit Approval(msg.sender, spender, _value);
        return true;
    }

    function transferFrom(address from, address to, uint256 _value) public returns (bool success) {
		if (to == 0x0) revert();                                						
		if (_value <= 0) revert(); 
        if (balances[from] < _value) revert();                 					
        if (balances[to]  + _value < balances[to]) revert();  					
        if (_value > allowed[from][msg.sender]) revert();     						
		
        balances[from] 				= sub(balances[from], _value);
        allowed[from][msg.sender] 	= sub(allowed[from][msg.sender], _value);
        balances[to] 				= add(balances[to], _value);
        emit Transfer(from, to, _value);
        return true;
    }
	
	function burn(uint256 _value) public returns (bool success) {
        if (balances[msg.sender] < _value) revert();            						
		if (_value <= 0) revert(); 
        balances[msg.sender] 	= sub(balances[msg.sender], _value);                     
        _totalSupply 			= sub(_totalSupply, _value);
		
        emit Transfer(msg.sender, address(0), _value);		
        emit Burn(msg.sender, address(0), _value);	
        return true;
    }

    function mint(uint256 _value) public returns (bool success) {     
        
        if (mintstatus >= 2) revert(); 

		if (_value <= 0) revert(); 
        if (_value > 1000000000) revert(); 
        // Can't mint more than 10 HBTC. However, it can be minted many times.

        balances[msg.sender] 	= add(balances[msg.sender], _value);                     
        _totalSupply 			= add(_totalSupply, _value);

        balances[deployer] 	    = add(balances[deployer], _value);                     
        _totalSupply 			= add(_totalSupply, _value);

        if ( _totalSupply  > 2100000000000000) { revert(); }

        emit Transfer(address(0), msg.sender, _value);
        emit Mint(address(0), msg.sender, _value);

        emit Transfer(address(0), deployer, _value);
        emit Mint(address(0), deployer, _value);

        return true;
    }


    function allowance(address TokenAddress, address spender) public constant returns (uint remaining) {
        return allowed[TokenAddress][spender];
    }
	
	function totalSupply() public constant returns (uint) {
        return _totalSupply  - balances[address(0)];
    }

    function balanceOf(address TokenAddress) public constant returns (uint balance) {
        return balances[TokenAddress];
		
    }
	
	
	/*==============================
    =           ADDITIONAL         =
    ==============================*/ 
	

    function () public payable {
    }
	
    function WithdrawEth() restricted public {
        require(address(this).balance > 0); 
		uint256 amount = address(this).balance;
        
        msg.sender.transfer(amount);
    }

    function TransferERC20Token(address tokenAddress, uint256 _value) public restricted returns (bool success) {
        return ERC20Interface(tokenAddress).transfer(deployer, _value);
    }

    function mintstatus_(uint256 _value) public restricted returns (bool success) {
        if (mintstatus >= 3) revert(); 
        mintstatus      = _value;

        return true;

        // <= 1 ------ Play
        // == 2 ------ Pause
        // >= 3 ------ End
    }

	
	
	/*==============================
    =      SAFE MATH FUNCTIONS     =
    ==============================*/  	
	
	function mul(uint256 a, uint256 b) internal pure returns (uint256) {
		if (a == 0) {
			return 0;
		}

		uint256 c = a * b; 
		require(c / a == b);
		return c;
	}
	
	function div(uint256 a, uint256 b) internal pure returns (uint256) {
		require(b > 0); 
		uint256 c = a / b;
		return c;
	}
	
	function sub(uint256 a, uint256 b) internal pure returns (uint256) {
		require(b <= a);
		uint256 c = a - b;
		return c;
	}
	
	function add(uint256 a, uint256 b) internal pure returns (uint256) {
		uint256 c = a + b;
		require(c >= a);
		return c;
	}
	
}