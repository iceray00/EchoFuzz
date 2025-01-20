/**
 *Submitted for verification at Etherscan.io on 2024-09-11
*/

pragma solidity ^0.4.24;
contract Marriage {

 mapping (address => uint) balances;
 address wife = address(0); // dummy address

 function () payable {
 balances[wife] += msg.value / 2;
 }
}