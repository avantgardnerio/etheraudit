const Web3 = require('web3');

const web3 = new Web3(new Web3.providers.HttpProvider("http://localhost:8545"));

console.log('web3.eth.accounts=', web3.eth.accounts)