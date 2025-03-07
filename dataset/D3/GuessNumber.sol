/**
 *Submitted for verification at Etherscan.io on 2024-12-15
*/

pragma solidity ^0.4.20;

contract GuessNumber {
    uint256 private secretNumber = (uint256(keccak256(now)) % 10) + 1;
    uint256 public lastPlayed;
    address public owner;

    struct Guess {
        address player;
        uint256 number;
    }

    Guess[] public guesses;

    function GuessNumber() public payable {
        owner = msg.sender;
    }

    function guessNumber(uint256 number) public {
        require(number <= 10);
        Guess guess;
        guess.player = msg.sender;
        guess.number = number;
        guesses.push(guess);
        lastPlayed = now;

        if (number == secretNumber) {
            msg.sender.transfer(address(this).balance);
        }
    }

    function kill() public {
        if (msg.sender == owner) {
            selfdestruct(msg.sender);
        }
    }
}