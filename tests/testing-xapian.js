exports.runTests = function (name, tests, sync, _done) {
  console.log(name);
  var objects = {};
  runTest(0);
  function runTest(indTst)
  {
    if (indTst == tests.length) { 
      if (_done) _done();
      return; 
    }
    if ('action' in tests[indTst]) {
      try {
       tests[indTst].action(objects, sync, function(err) {
          if (err) {
            if (tests[indTst].fatal) {
              console.log('  fatal ' + tests[indTst].name + ' - ' + err);
              if (_done) _done();
            } else {
              console.log('  fail  ' + tests[indTst].name);
              runTest(indTst + 1);
            }
          } else {
            console.log('  ok    ' + tests[indTst].name);
            runTest(indTst + 1);
          }
        });
      } catch (ex) {
         if (tests[indTst].fatal) {
           console.log('  fatal ' + tests[indTst].name + ' - ' + ex.message); 
           if (_done) _done();
         } else {
           console.log('  fail  ' + tests[indTst].name);
           runTest(indTst + 1);
         }
      }
    }
    else {
      if (sync) {
        var aResult, aErr = null;
        try {
          aResult = objects[tests[indTst].obj][tests[indTst].method].apply(objects[tests[indTst].obj], tests[indTst].parameters);
        } catch (ex) {
          aErr = ex.message;
        }
        tests[indTst].result(aErr, aResult, function(result) {
          console.log((result ? '  ok    ' : '  fail  ') + tests[indTst].name); 
          runTest(indTst + 1); 
        });
      } else {
        var aAlteredParameters = [];
        for (var i=0; i < tests[indTst].parameters.length; i++ ) aAlteredParameters.push(tests[indTst].parameters[i]);
        aAlteredParameters.push(function(err, result){
          tests[indTst].result(err, result, function(result) {
            console.log((result ? '  ok    ' : '  fail  ') + tests[indTst].name); 
            runTest(indTst + 1); 
          });          
        });
        try {
          objects[tests[indTst].obj][tests[indTst].method].apply(objects[tests[indTst].obj], aAlteredParameters);
        } catch (ex) {
          tests[indTst].result(ex.message, null, function(result) {
            console.log((result ? '  ok    ' : '  fail  ') + tests[indTst].name); 
            runTest(indTst + 1); 
          });
        }
      }
    }
  }
}
