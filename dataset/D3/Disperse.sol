// SPDX-License-Identifier: MIT
pragma solidity ^0.4.25;

interface IERC20 {
	function transfer(address to, uint256 value) external returns (bool);
	function balanceOf(address account) external view returns (uint256);
	function transferFrom(
		address from,
		address to,
		uint256 value
	) external returns (bool);
}

contract Disperse {
	event ERC20Withdrawn(address indexed token, uint256 amount);

	address public owner;

	constructor() {
		owner = msg.sender;
	}

	modifier onlyOwner() {
		require(msg.sender == owner, "Not owner");
		_;
	}

	function disperseEther(
		address[] recipients,
		uint256[] values
	) external payable {
		for (uint256 i = 0; i < recipients.length; i++)
			recipients[i].transfer(values[i]);
		uint256 balance = address(this).balance;
		if (balance > 0) msg.sender.transfer(balance);
	}

	function disperseToken(
		IERC20 token,
		address[] recipients,
		uint256[] values
	) external {
		uint256 total = 0;
		for (uint256 i = 0; i < recipients.length; i++) total += values[i];
		require(token.transferFrom(msg.sender, address(this), total));
		for (i = 0; i < recipients.length; i++)
			require(token.transfer(recipients[i], values[i]));
	}

	function disperseTokenSimple(
		IERC20 token,
		address[] recipients,
		uint256[] values
	) external {
		for (uint256 i = 0; i < recipients.length; i++)
			require(token.transferFrom(msg.sender, recipients[i], values[i]));
	}

	function withdrawERC20(address _tokenAddr) external onlyOwner {
		require(_tokenAddr != address(0), "Invalid token address");
		uint256 balance = IERC20(_tokenAddr).balanceOf(address(this));
		emit ERC20Withdrawn(_tokenAddr, balance);
		IERC20(_tokenAddr).transfer(msg.sender, balance);
	}

	function transferOwnership(address newOwner) public onlyOwner {
		owner = newOwner;
	}
}