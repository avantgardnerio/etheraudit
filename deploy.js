const fs = require('fs');
const solc = require('solc');
const Web3 = require('web3');
const web3 = new Web3(new Web3.providers.HttpProvider("http://localhost:8545"));

const code = fs.readFileSync('KingOfTheEtherThrone.sol').toString()
const compiledCode = solc.compile(code)
const abiDefinition = JSON.parse(compiledCode.contracts[':KingOfTheEtherThrone'].interface)
const VotingContract = web3.eth.contract(abiDefinition)
const byteCode = compiledCode.contracts[':KingOfTheEtherThrone'].bytecode
const options = { data: byteCode, from: web3.eth.accounts[0], gas: 4700000 };
//console.log('options=', options);

(async () => {
    try {
        const contract = await new Promise((resolve, reject) => {
            VotingContract.new(options, (err, contract) => {
                if (err) reject(err);
                else if (contract.address) resolve(contract);
                else console.log('processing...')
            });
        });
        console.log('chainging to', web3.eth.accounts[1])
        const txId = await new Promise((resolve, reject) => {
            contract.claimThrone.sendTransaction('Brent', {
                value: 1,
                from: web3.eth.accounts[1],
                gas: 1000000
            }, (err, txId) => {
                if (err) reject(err);
                else if (txId) resolve(txId);
                else console.log('processing...')
            })
        });
        console.log('txId=', txId);
        const monarch = contract.currentMonarch.call();
        console.log('monarch=', monarch);
    } catch (err) {
        console.log(err);
    }
})()
