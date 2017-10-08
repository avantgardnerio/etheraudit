# EtherAudit

Ethereum Contract Auditor

## About

Use fuzzing and static code analysis to find bugs in ethereum smart contracts.

# Prereqs
- CMake >= 3.7
- g++ >= 7.0

## Setup

```bash
yarn install
cd src && mkdir -p build && cd build && cmake ..
make
```

## Running

```bash
yarn server # start a testrpc server (mock ethereum node)
yarn deploy # compile the contract and deploy to ethereum
```

## References

* [ethereum full stack voting tutorial](https://medium.com/@mvmurthy/full-stack-hello-world-voting-ethereum-dapp-tutorial-part-1-40d2d0d807c2)
