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
console.log('options=', options);
VotingContract.new(options, function (err, contract) {
    console.log('done, ', contract.address);
    const contractInstance = VotingContract.at(contract.address);
    console.log('contractInstance, ', contractInstance);
});
