/**
 *Submitted for verification at Etherscan.io on 2024-09-03
*/

pragma solidity ^0.4.25;

contract contractofmine {

    // naive recursion
    function sum(uint n) view returns(uint) {
        return n == 0 ? 0 :
          n + sum(n-1);
    }

    // tail-recursion
    function sumtailHelper(uint n, uint acc) private view returns(uint) {
        return n == 0 ? acc :
          sumtailHelper(n-1, acc + n);
    }
    function sumtail(uint n) view returns(uint) {
        return sumtailHelper(n, 0);
    }
}