var xapiantesting = require('./testing-xapian');
var fs = require('fs');
var path = require('path');

var aAllFiles = fs.readdirSync(path.dirname(module.filename));
var aAllTestFiles = [];
for (var i=0; i < aAllFiles.length; i++) {
  if (aAllFiles[i].match(/^test_.*\.js$/g))
    if (process.argv.length < 3 || aAllFiles[i].match(process.argv[2]))
      aAllTestFiles.push(aAllFiles[i]);
}

var aResults = [];

doTest(0, true);
function doTest(i, sync) {
  if(i >= aAllTestFiles.length) {
    console.log('Summary: ok fail fatal   name');
    for (var ir = 0; ir < aResults.length; ir++) {
      console.log(aResults[ir][1]+' '+aResults[ir][2]+' '+aResults[ir][3]+'   '+aResults[ir][0]);
    }
    return;
  }
  var aTest = require('./' + aAllTestFiles[i])
  xapiantesting.runTests(aTest.name + (sync ? ' - sync' : ' - async'), aTest.tests, sync, function (ok, fail, fatal) {
    console.log();
    aResults.push([aTest.name + (sync ? ' - sync' : ' - async'), ok, fail, fatal]);
    doTest(i + !sync, !sync);
  });
}
