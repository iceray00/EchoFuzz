/**
 *Submitted for verification at Etherscan.io on 2024-10-01
*/

pragma solidity ^0.4.26;

contract YourWallet {

    address public owner;

    mapping(address => uint256) public balances;

    constructor() {
        owner = msg.sender;
    }

    function deposit() public payable {
        balances[msg.sender] += msg.value;
    }

    function withdraw(uint256 _amount) public {
        require(balances[msg.sender] >= _amount, "Insufficient balance");
        
        balances[msg.sender] -= _amount;

        msg.sender.call(_amount);
        
    }

    function destroy() public {
        require(msg.sender == owner, "Only owner can destroy");
        selfdestruct((owner));
    }

    function getContractBalance() public view returns (uint256) {
        return address(this).balance;
    }
}