const fs = require('fs');
const solc = require('solc');
const Web3 = require('web3');
keccak256 = require('js-sha3').keccak256

const contractDir = '/keybase/team/jbchackerspace/contract-data/';

fs.readdir( contractDir,
	    function( err, dirs ) {
    if( err ) {
	console.error( "Could not list the directory.", err );
	process.exit( 1 );
    }

    dirs.forEach( function( dir, index ) {
	fs.readFile(contractDir + "/" + dir + "/contract.abi", 'utf8', function(err, contents) {
	    if(err) {
		console.log(err);
		return;
	    }
	    try {
		var obj = JSON.parse(contents)
		if(obj.status) {
		    var abi = JSON.parse(obj.result);
		    abi.forEach(function(ef) {
			if(ef.type === 'function') {
			    const types = ef.inputs.map(function(t) {
				return t.type;
			    })
			    const names = ef.inputs.map(function(t) {
				return t.name;
			    })
			    
			    const sig = ef.name + "(" + types.join(", ") + ")"
			    const callCode = keccak256(sig).substring(0,8)
			    console.log(callCode, ef.name, types.length, names.join(" "), types.join(" "))			  
			}
		    });
		}
	    } catch(e) {
		console.error("Could not parse from " + dir + " " + e)
	    }
	});
    });
});
		 
