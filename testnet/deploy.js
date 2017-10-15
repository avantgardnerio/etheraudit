const fs = require('fs');
const solc = require('solc');
const Web3 = require('web3');
const web3 = new Web3(new Web3.providers.HttpProvider("http://127.0.0.1:8545"));

const code = fs.readFileSync('KingOfTheEtherThrone.sol').toString()
const compiledCode = solc.compile(code)
const abiDefinition = JSON.parse(compiledCode.contracts[':KingOfTheEtherThrone'].interface)
const VotingContract = web3.eth.contract(abiDefinition)
const byteCode = compiledCode.contracts[':KingOfTheEtherThrone'].bytecode
const from = web3.eth.accounts[0].substring(2);
const options = {
    data: '0x' + byteCode,
    from: web3.eth.accounts[0],
    gas: 2100000//4700000
};
console.log('options=', options);

console.log('balance=', web3.eth.getBalance(web3.eth.accounts[0]).toString());

(async () => {
    try {
        const contract = await new Promise((resolve, reject) => {
            VotingContract.new(options, (err, contract) => {
                if (err) reject(err);
                else if (contract.address) resolve(contract);
                else console.log('processing...')
            });
        });
        console.log('contract address=', contract.address);

        const takeThrone = (msg, amount, address) => {
            console.log('chainging to', address)
            return new Promise((resolve, reject) => {
                contract.claimThrone.sendTransaction(msg, {
                    value: amount,
                    from: address,
                    gas: 1000000
                }, (err, txId) => {
                    if (err) reject(err);
                    else if (txId) resolve(txId);
                    else console.log('processing...')
                })
            })
        }

        await takeThrone('Brent', 1, web3.eth.accounts[0]);
        let monarch = contract.currentMonarch.call();
        let balance = web3.fromWei(web3.eth.getBalance(web3.eth.accounts[0]));
        console.log(`monarch=${monarch} brent=${balance}`);

        // await takeThrone('Justin', 2, web3.eth.accounts[2]);
        // monarch = contract.currentMonarch.call();
        // console.log('monarch=', monarch);

    } catch (err) {
        console.log(err);
    }
})()
