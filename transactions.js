const fs = require('fs');
const solc = require('solc');
const Web3 = require('web3');
const web3 = new Web3(new Web3.providers.HttpProvider("http://127.0.0.1:8545"));

const accounts = {};

for (let i = 1000000; i < web3.eth.syncing.currentBlock; i++) {
    const block = web3.eth.getBlock(i);
    accounts[block.miner] = accounts[block.miner] | 0;
    console.log(`block=${i} trans=${block.transactions}`);
    if (block.transactions.length > 0) {
        for (let txnId of block.transactions) {
            const txn = web3.eth.getTransaction(txnId);
            accounts[block.from] = accounts[block.from] | 0;
            accounts[block.to] = accounts[block.to] | 0;
            console.log(txn);
        }
    }
}