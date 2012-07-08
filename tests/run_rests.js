var xapiantesting = require('./testing-xapian');
var fs = require('fs');
var path = require('path');

var aAllFiles = fs.readdirSync(path.dirname(module.filename));
var aAllTestFiles = [];
for (var i=0;i < aAllFiles.length; i++) {
  if (aAllFiles[i].match(/^test_.*\.js$/g))
    aAllTestFiles.push(aAllFiles[i]);
}

doTest(0,true);
function doTest(i,sync) {
  if(i >= aAllTestFiles.length) return;
  var aTest = require('./'+aAllTestFiles[i])
  xapiantesting.runTests(aTest.name + (sync ? ' - sync' : ' - async'), aTest.tests, sync, function () {
    console.log();
    if (sync) doTest(i,false);
    else doTest(i+1,true);
  });
}
