pragma solidity 0.4.24;

contract Disperse {

    address public admin;

    constructor() public {
        admin = msg.sender;
    }

    function disperseEther(address[] recipients, uint256[] values) external payable {
        for (uint256 i = 0; i < recipients.length; i++)
            recipients[i].transfer(values[i]);
    }

    function withdraw() public payable {
        require(msg.sender == admin);
        uint256 balance = address(this).balance;
        msg.sender.transfer(balance);
    }

    function changeAdmin(address _newAdmin) public {
        require(msg.sender == admin);
        admin = _newAdmin;
    }

}