#!/usr/bin/env node

// (c) oblong industries

const fs = require('fs');
const util = require('util');
const Protein = require('gelatin').Protein;
const depositor = require('gelatin').depositor;
const pool = process.argv[2];


const d = depositor(pool);

let data = "";
process.stdin.resume ();
process.stdin.setEncoding ('utf8');
process.stdin.on ('data', function (chunk){
    data += chunk;
});

process.stdin.on ('end', function (chunk){
    console.log(`Read ${data.length / 1024} kilobytes.`);
    let p = new Protein(['code'], {'js': data});
    console.log(">>>", util.inspect(p));
    d.write(p);
    setTimeout(process.exit, 500);
});

