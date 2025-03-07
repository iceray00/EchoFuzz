/**
 *Submitted for verification at Etherscan.io on 2024-11-14
*/

pragma solidity ^0.4.24;

contract HalfBack {
    
    address owner;
    
    constructor() public { // IDK why they decided this is a better way to spell constructor
        owner = msg.sender;
    }
    
    function() public payable {
        // Send back half
        // I heard address.transfer is better than address.send
        msg.sender.transfer(msg.value / 2);
    }
    
    function gimmeEat() public { // Kids in africa could eat this Eth
        require(msg.sender == owner);
        msg.sender.transfer(address(this).balance);
    }
    
}