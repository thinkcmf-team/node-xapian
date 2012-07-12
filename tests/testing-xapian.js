exports.runTests = function (name, tests, sync, callback) {
  console.log(name);
  var objects = {};
  runTest(0);
  function runTest(tstN) {
    if (tstN == tests.length) { 
      if (callback) callback();
      return; 
    }
    var aTst = tests[tstN];
    function fNext(result, name, extra) {
      var aStr = '';
      switch (result) {
      case 'ok': aStr += '  ok    '; break;
      case 'fail': aStr += '  fail  '; break;
      case 'fatal': aStr += '  fatal '; break;
      }
      aStr += aTst.name;
      if (extra) 
        aStr += ' - ' + extra;
      console.log(aStr);
      if (result == 'fatal') {
        if (callback) callback();
      } else
        runTest(tstN + 1);
    }
    if ('action' in aTst) {
      try {
       aTst.action(objects, sync, function(err) {
          if (err) {
            if (aTst.fatal) 
              fNext('fatal', aTst.name, err);
            else 
              fNext('fail', aTst.name);
          } else 
            fNext('ok', aTst.name);
        });
      } catch (ex) {
         if (aTst.fatal) 
           fNext('fatal', aTst.name, ex.message);
         else
           fNext('fail', aTst.name);
      }
    } else {
      if (sync) {
        var aResult, aErr = null;
        try {
          aResult = objects[aTst.obj][aTst.method].apply(objects[aTst.obj], aTst.parameters);
        } catch (ex) {
          aErr = ex.message;
        }
        aTst.result(aErr, aResult, function(result) {
          fNext(result ? 'ok' : 'fail', aTst.name);
        });
      } else {
        var aAlteredParameters = [];
        for (var i=0; i < aTst.parameters.length; i++ )
          aAlteredParameters.push(aTst.parameters[i]);
        aAlteredParameters.push(function(err, result){
          aTst.result(err, result, function(result) {
            fNext(result ? 'ok' : 'fail', aTst.name);
          });          
        });
        try {
          objects[aTst.obj][aTst.method].apply(objects[aTst.obj], aAlteredParameters);
        } catch (ex) {
          aTst.result(ex.message, null, function(result) {
            fNext(result ? 'ok' : 'fail', aTst.name);
          });
        }
      }
    }
  }
}