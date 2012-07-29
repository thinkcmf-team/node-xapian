var lXapian = require('../xapian-binding');

exports.name = "WritableDatabase tests from perl";
exports.tests = [
  {
    name: 'wdb = new WritableDatabase',
    fatal: true,
    action: function(objects, sync, fn) { 
      if (sync) {
        objects.wdb = new lXapian.WritableDatabase('testdb-writabledatabase-sync', lXapian.DB_CREATE_OR_OVERWRITE); 
        fn(null); 
      } else {
        new lXapian.WritableDatabase('estdb-writabledatabase-async', lXapian.DB_CREATE_OR_OVERWRITE, function(err, result) {
          objects.wdb = result;
          fn(err);
        });
      }
    }
  }, {
    name: 'indexing documents',
    fatal: true,
    action: function(objects, sync, fn) {
      function addDocuments(a, callback) {
        if (a > 1003) {
          callback(null);
          return;
        }
        var doc = new lXapian.Document();
        var aNum = ["three", "four", "five"];
        if (a > 1000) {
          doc.set_data("test " + aNum[a - 1001]);
          doc.add_posting("test", 0);
          doc.add_posting(aNum[a - 1001], 1);
          doc.add_value(0, aNum[a - 1001]);
        } else {
          doc.set_data("test " + a);
          doc.add_posting("test", 0);
          doc.add_posting(a + "", 1);
          doc.add_value(0, a + "");
        }
        if (sync) {
          objects.wdb.add_document(doc);
          addDocuments(a+1, callback);
        } else {
          objects.wdb.add_document(doc, function(err, result){
            if (err)
              callback(err);
            else 
              addDocuments(a+1, callback);
          });
        }
      }
      addDocuments(1,fn);
    }
  }, {
    name: 'wdb.commit',
    obj: 'wdb',
    method: 'commit',
    parameters: [],
    result: function(err, result, fn) { fn(err == null); }
  }, {
    name: 'wdb.get_doccount',
    obj: 'wdb',
    method: 'get_doccount',
    parameters: [],
    result: function(err, result, fn) { fn(err == null && result == 1003); }
  }, {
    name: 'wdb.term_exists',
    obj: 'wdb',
    method: 'term_exists',
    parameters: ["six"],
    result: function(err, result, fn) { fn(err == null && result == false); }
  }, {
    name: 'wdb.get_document',
    obj: 'wdb',
    method: 'get_document',
    parameters: [500],
    result: function(err, result, fn) { fn(err == null && result.get_data() == "test 500"); }
  }, {
    name: 'wdb.replace_document by docid',
    fatal: true,
    action: function(objects, sync, fn) {
      var doc = new lXapian.Document();
      doc.set_data("test six");
      doc.add_posting("test", 0);
      doc.add_posting("six", 1);
      doc.add_value(0, "six");
      if (sync) {
        objects.wdb.replace_document(500, doc);
        fn(null); 
      } else {
        objects.wdb.replace_document(500, doc, fn);
      }
    }
  }, {
    name: 'wdb.commit',
    obj: 'wdb',
    method: 'commit',
    parameters: [],
    result: function(err, result, fn) { fn(err == null); }
  }, {
    name: 'wdb.term_exists',
    obj: 'wdb',
    method: 'term_exists',
    parameters: ["six"],
    result: function(err, result, fn) { fn(err == null && result == true); }
  }, {
    name: 'wdb.get_document',
    obj: 'wdb',
    method: 'get_document',
    parameters: [500],
    result: function(err, result, fn) { fn(err == null && result.get_data() == "test six"); }
  }, {
    name: 'wdb.term_exists',
    obj: 'wdb',
    method: 'term_exists',
    parameters: ["seven"],
    result: function(err, result, fn) { fn(err == null && result == false); }
  }, {
    name: 'wdb.term_exists',
    obj: 'wdb',
    method: 'term_exists',
    parameters: ["five"],
    result: function(err, result, fn) { fn(err == null && result == true); }
  }, {
    name: 'wdb.get_termfreq',
    obj: 'wdb',
    method: 'get_termfreq',
    parameters: ["seven"],
    result: function(err, result, fn) { fn(err == null && result == 0); }
  }, {
    name: 'wdb.get_termfreq',
    obj: 'wdb',
    method: 'get_termfreq',
    parameters: ["five"],
    result: function(err, result, fn) { fn(err == null && result == 1); }
  }, {
    name: 'wdb.replace_document by term',
    fatal: true,
    action: function(objects, sync, fn) {
      var doc = new lXapian.Document();
      doc.set_data("test seven");
      doc.add_posting("test", 0);
      doc.add_posting("seven", 1);
      doc.add_value(0, "seven");
      if (sync) {
        objects.wdb.replace_document("five", doc);
        fn(null); 
      } else {
        objects.wdb.replace_document("five", doc, fn);
      }
    }
  }, {
    name: 'wdb.commit',
    obj: 'wdb',
    method: 'commit',
    parameters: [],
    result: function(err, result, fn) { fn(err == null); }
  }, {
    name: 'wdb.term_exists',
    obj: 'wdb',
    method: 'term_exists',
    parameters: ["seven"],
    result: function(err, result, fn) { fn(err == null && result == true); }
  }, {
    name: 'wdb.term_exists',
    obj: 'wdb',
    method: 'term_exists',
    parameters: ["five"],
    result: function(err, result, fn) { fn(err == null && result == false); }
  }, {
    name: 'wdb.get_termfreq',
    obj: 'wdb',
    method: 'get_termfreq',
    parameters: ["seven"],
    result: function(err, result, fn) { fn(err == null && result == 1); }
  }, {
    name: 'wdb.get_termfreq',
    obj: 'wdb',
    method: 'get_termfreq',
    parameters: ["five"],
    result: function(err, result, fn) { fn(err == null && result == 0); }
  }, {
    name: 'wdb.get_termfreq',
    obj: 'wdb',
    method: 'get_termfreq',
    parameters: ["test"],
    result: function(err, result, fn) { fn(err == null && result == 1003); }
  }, {
    name: 'wdb.get_termfreq',
    obj: 'wdb',
    method: 'get_termfreq',
    parameters: ["eight"],
    result: function(err, result, fn) { fn(err == null && result == 0); }
  }, {
    name: 'wdb.replace_document by new term',
    fatal: true,
    action: function(objects, sync, fn) {
      var doc = new lXapian.Document();
      doc.set_data("test eight");
      doc.add_posting("test", 0);
      doc.add_posting("eight", 1);
      doc.add_value(0, "eight");
      if (sync) {
        objects.wdb.replace_document("eight", doc);
        fn(null); 
      } else {
        objects.wdb.replace_document("eight", doc, fn);
      }
    }
  }, {
    name: 'wdb.commit',
    obj: 'wdb',
    method: 'commit',
    parameters: [],
    result: function(err, result, fn) { fn(err == null); }
  }, {
    name: 'wdb.get_doccount',
    obj: 'wdb',
    method: 'get_doccount',
    parameters: [],
    result: function(err, result, fn) { fn(err == null && result == 1004); }
  }, {
    name: 'wdb.get_termfreq',
    obj: 'wdb',
    method: 'get_termfreq',
    parameters: ["test"],
    result: function(err, result, fn) { fn(err == null && result == 1004); }
  }, {
    name: 'wdb.get_termfreq',
    obj: 'wdb',
    method: 'get_termfreq',
    parameters: ["eight"],
    result: function(err, result, fn) { fn(err == null && result == 1); }
  }, {
    name: 'wdb.replace_document multiple',
    fatal: true,
    action: function(objects, sync, fn) {
      var doc = new lXapian.Document();
      doc.set_data("test nine");
      doc.add_posting("test", 0);
      doc.add_posting("nine", 1);
      doc.add_value(0, "nine");
      if (sync) {
        objects.wdb.replace_document("test", doc);
        fn(null); 
      } else {
        objects.wdb.replace_document("test", doc, fn);
      }
    }
  }, {
    name: 'wdb.commit',
    obj: 'wdb',
    method: 'commit',
    parameters: [],
    result: function(err, result, fn) { fn(err == null); }
  }, {
    name: 'wdb.get_doccount',
    obj: 'wdb',
    method: 'get_doccount',
    parameters: [],
    result: function(err, result, fn) { fn(err == null && result == 1); }
  }, {
    name: 'wdb.get_document',
    obj: 'wdb',
    method: 'get_document',
    parameters: [1],
    result: function(err, result, fn) { fn(err == null && result.get_data() == "test nine"); }
  }, {
    name: 'indexing documents',
    fatal: true,
    action: function(objects, sync, fn) {
      function addDocuments(a, callback) {
        if (a >= 5) {
          callback(null);
          return;
        }
        var doc = new lXapian.Document();
        var aNum = ["one", "two", "three", "four", "five"];

        doc.set_data("test " + aNum[a]);
        doc.add_posting("test", 0);
        doc.add_posting(aNum[a], 1);
        doc.add_value(0, aNum[a]);

        if (sync) {
          objects.wdb.add_document(doc);
          addDocuments(a+1, callback);
        } else {
          objects.wdb.add_document(doc, function(err, result){
            if (err)
              callback(err);
            else 
              addDocuments(a+1, callback);
          });
        }
      }
      addDocuments(0,fn);
    }
  }, {
    name: 'wdb.commit',
    obj: 'wdb',
    method: 'commit',
    parameters: [],
    result: function(err, result, fn) { fn(err == null); }
  }, {
    name: 'wdb.get_doccount',
    obj: 'wdb',
    method: 'get_doccount',
    parameters: [],
    result: function(err, result, fn) { fn(err == null && result == 6); }
  }, {
    name: 'wdb.get_lastdocid',
    obj: 'wdb',
    method: 'get_lastdocid',
    parameters: [],
    result: function(err, result, fn) { fn(err == null && result == 1009); }
  }, {
    name: 'wdb.get_document',
    obj: 'wdb',
    method: 'get_document',
    parameters: [1009],
    result: function(err, result, fn) { fn(err == null && result.get_value(0) == "five"); }
  }, {
    name: 'wdb.term_exists',
    obj: 'wdb',
    method: 'term_exists',
    parameters: ["five"],
    result: function(err, result, fn) { fn(err == null && result == true); }
  }, {
    name: 'wdb.delete_document',
    obj: 'wdb',
    method: 'delete_document',
    parameters: [1009],
    result: function(err, result, fn) { fn(err == null); }
  }, {
    name: 'wdb.commit',
    obj: 'wdb',
    method: 'commit',
    parameters: [],
    result: function(err, result, fn) { fn(err == null); }
  }, {
    name: 'wdb.get_doccount',
    obj: 'wdb',
    method: 'get_doccount',
    parameters: [],
    result: function(err, result, fn) { fn(err == null && result == 5); }
  }, {
    name: 'wdb.term_exists',
    obj: 'wdb',
    method: 'term_exists',
    parameters: ["five"],
    result: function(err, result, fn) { fn(err == null && result == false); }
  }, {
    name: 'wdb.term_exists',
    obj: 'wdb',
    method: 'term_exists',
    parameters: ["three"],
    result: function(err, result, fn) { fn(err == null && result == true); }
  }, {
    name: 'wdb.get_termfreq',
    obj: 'wdb',
    method: 'get_termfreq',
    parameters: ["three"],
    result: function(err, result, fn) { fn(err == null && result == 1); }
  }, {
    name: 'wdb.delete_document',
    obj: 'wdb',
    method: 'delete_document',
    parameters: ["three"],
    result: function(err, result, fn) { fn(err == null); }
  }, {
    name: 'wdb.commit',
    obj: 'wdb',
    method: 'commit',
    parameters: [],
    result: function(err, result, fn) { fn(err == null); }
  }, {
    name: 'wdb.get_doccount',
    obj: 'wdb',
    method: 'get_doccount',
    parameters: [],
    result: function(err, result, fn) { fn(err == null && result == 4); }
  }, {
    name: 'wdb.term_exists',
    obj: 'wdb',
    method: 'term_exists',
    parameters: ["three"],
    result: function(err, result, fn) { fn(err == null && result == false); }
  }, {
    name: 'wdb.get_termfreq',
    obj: 'wdb',
    method: 'get_termfreq',
    parameters: ["three"],
    result: function(err, result, fn) { fn(err == null && result == 0); }
  }, {
    name: 'wdb.term_exists',
    obj: 'wdb',
    method: 'term_exists',
    parameters: ["test"],
    result: function(err, result, fn) { fn(err == null && result == true); }
  }, {
    name: 'wdb.get_termfreq',
    obj: 'wdb',
    method: 'get_termfreq',
    parameters: ["test"],
    result: function(err, result, fn) { fn(err == null && result == 4); }
  }, {
    name: 'wdb.delete_document',
    obj: 'wdb',
    method: 'delete_document',
    parameters: ["test"],
    result: function(err, result, fn) { fn(err == null); }
  }, {
    name: 'wdb.commit',
    obj: 'wdb',
    method: 'commit',
    parameters: [],
    result: function(err, result, fn) { fn(err == null); }
  }, {
    name: 'wdb.get_doccount',
    obj: 'wdb',
    method: 'get_doccount',
    parameters: [],
    result: function(err, result, fn) { fn(err == null && result == 0); }
  }, {
    name: 'wdb.term_exists',
    obj: 'wdb',
    method: 'term_exists',
    parameters: ["test"],
    result: function(err, result, fn) { fn(err == null && result == false); }
  }, {
    name: 'wdb.get_termfreq',
    obj: 'wdb',
    method: 'get_termfreq',
    parameters: ["test"],
    result: function(err, result, fn) { fn(err == null && result == 0); }
  }, {
    name: 'wdb = new WritableDatabase twice',
    fatal: false,
    action: function(objects, sync, fn) { 
      if (sync) {
        var aErr = false;
        try {
          new lXapian.WritableDatabase('testdb-writabledatabase-sync', lXapian.DB_CREATE_OR_OPEN); 
        } catch (ex) {
          aErr = true;
          fn(null); 
        }
        if (!aErr)
          fn("Opening database twice"); 
      } else {
        new lXapian.WritableDatabase('estdb-writabledatabase-async', lXapian.DB_CREATE_OR_OPEN, function(err, result) {
          if (err)
            fn(null);
          else
            fn("Opening database twice"); 
        });
      }
    }
  }, {
    name: 'wdb.close',
    obj: 'wdb',
    method: 'close',
    parameters: [],
    result: function(err, result, fn) { fn(err == null); }
  }
]
