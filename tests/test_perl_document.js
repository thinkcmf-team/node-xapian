var xapian = require('../xapian-binding');
var sys = require('sys');

exports.name = "Document tests from perl";
exports.tests = [
  {
    name: 'doc1 = new Document',
    fatal: true,
    action: function(objects, sync, fn) { objects.doc1 = new xapian.Document(); fn(null); }
  }, {
    name: 'doc1.set_data hello world',
    obj: 'doc1',
    method: 'set_data',
    parameters: [ "hello world" ],
    result: function(err, result, fn) { fn(err == null); }
  }, {
    name: 'doc1.get_data hello world',
    obj: 'doc1',
    method: 'get_data',
    parameters: [],
    result: function(err, result, fn) { fn(err == null && result == 'hello world'); }
  }, {
    name: 'doc1.add_value fudge',
    obj: 'doc1',
    method: 'add_value',
    parameters: [ 1, "fudge" ],
    result: function(err, result, fn) { fn(err == null); }
  }, {
    name: 'doc1.add_value chocolate',
    obj: 'doc1',
    method: 'add_value',
    parameters: [ 2, "chocolate" ],
    result: function(err, result, fn) { fn(err == null); }
  }, {
    name: 'doc1.get_value fudge',
    obj: 'doc1',
    method: 'get_value',
    parameters: [ 1 ],
    result: function(err, result, fn) { fn(err == null && result === 'fudge'); }
  }, {
    name: 'doc1.get_docid',
    obj: 'doc1',
    method: 'get_docid',
    parameters: [],
    result: function(err, result, fn) { fn(err == null && result === 0); }
  }, {
    name: 'doc1.values',
    obj: 'doc1',
    method: 'values',
    parameters: [],
    result: function(err, result, fn) { 
      fn(err == null && result.length == 2 &&
      result[0].value == 'fudge' && result[0].valueno == 1 &&
      result[1].value == 'chocolate' && result[1].valueno == 2
      );
    }
  }, {
    name: 'doc1.remove_value 1',
    obj: 'doc1',
    method: 'remove_value',
    parameters: [ 1 ],
    result: function(err, result, fn) { fn(err == null); }
  }, {
    name: 'doc1.get_value 1',
    obj: 'doc1',
    method: 'get_value',
    parameters: [ 1 ],
    result: function(err, result, fn) { fn(err == null && result == ""); }
  }, {
    name: 'doc1.get_value 2',
    obj: 'doc1',
    method: 'get_value',
    parameters: [ 2 ],
    result: function(err, result, fn) { fn(err == null && result == "chocolate"); }
  }, {
    name: 'doc1.clear_values',
    obj: 'doc1',
    method: 'clear_values',
    parameters: [],
    result: function(err, result, fn) { fn(err == null); }
  }, {
    name: 'doc1.get_value 2',
    obj: 'doc1',
    method: 'get_value',
    parameters: [2],
    result: function(err, result, fn) { fn(err == null && result == ""); }
  }, {
    name: 'db = new WritableDatabase',
    fatal: true,
    action: function(objects, sync, fn) { 
      if (sync) {
        objects.db = new xapian.WritableDatabase('tmpdbs', xapian.DB_CREATE_OR_OVERWRITE); 
        fn(null); 
      } else {
        new xapian.WritableDatabase('tmpdbas', xapian.DB_CREATE_OR_OVERWRITE, function(err, result) {
          objects.db = result;
          fn(err);
        });
      }
    }
  }, {
    name: 'doc1.add_posting',
    obj: 'doc1',
    method: 'add_posting',
    parameters: [ "hello", 1, 100 ],
    result: function(err, result, fn) { fn(err == null); }
  }, {
    name: 'doc1.add_posting',
    obj: 'doc1',
    method: 'add_posting',
    parameters: [ "hello", 2 ],
    result: function(err, result, fn) { fn(err == null); }
  }, {
    name: 'db.add_document(doc1)',
    fatal: true,
    action: function(objects, sync, fn) { 
      if (sync) {
        objects.db.add_document(objects.doc1);
        fn(null); 
      } else {
        objects.db.add_document(objects.doc1, function(err, result) {
          fn(err);
        });
      }
    }
  }, {
    name: 'db.get_doclength',
    obj: 'db',
    method: 'get_doclength',
    parameters: [ 1 ],
    result: function(err, result, fn) { fn(err == null && result == 101); }
  }, {
    name: 'doc2 = new Document',
    fatal: true,
    action: function(objects, sync, fn) { objects.doc2 = new xapian.Document(); fn(null); }
  }, {
    name: 'doc2.add_posting',
    obj: 'doc2',
    method: 'add_posting',
    parameters: [ "goodbye", 1, 1 ],
    result: function(err, result, fn) { fn(err == null); }
  }, {
    name: 'doc2.add_posting',
    obj: 'doc2',
    method: 'add_posting',
    parameters: [ "goodbye", 2, 1 ],
    result: function(err, result, fn) { fn(err == null); }
  }, {
    name: 'doc2.remove_posting',
    obj: 'doc2',
    method: 'remove_posting',
    parameters: [ "goodbye", 2 ],
    result: function(err, result, fn) { fn(err == null); }
  }, {
    name: 'db.add_document(doc2)',
    fatal: true,
    action: function(objects, sync, fn) { 
      if (sync) {
        objects.db.add_document(objects.doc2);
        fn(null); 
      } else {
        objects.db.add_document(objects.doc2, function(err, result) {
          fn(err);
        });
      }
    }
  }, {
    name: 'db.get_doclength',
    obj: 'db',
    method: 'get_doclength',
    parameters: [ 2 ],
    result: function(err, result, fn) { fn(err == null && result == 1); }
  }, {
    name: 'doc3 = new Document',
    fatal: true,
    action: function(objects, sync, fn) { objects.doc3 = new xapian.Document(); fn(null); }
  }, {
    name: 'doc3.add_term',
    obj: 'doc3',
    method: 'add_term',
    parameters: [ "a", 100 ],
    result: function(err, result, fn) { fn(err == null); }
  }, {
    name: 'doc3.add_term',
    obj: 'doc3',
    method: 'add_term',
    parameters: [ "a" ],
    result: function(err, result, fn) { fn(err == null); }
  }, {
    name: 'db.add_document(doc3)',
    fatal: true,
    action: function(objects, sync, fn) { 
      if (sync) {
        objects.db.add_document(objects.doc3);
        fn(null); 
      } else {
        objects.db.add_document(objects.doc3, function(err, result) {
          fn(err);
        });
      }
    }
  }, {
    name: 'db.get_doclength',
    obj: 'db',
    method: 'get_doclength',
    parameters: [ 3 ],
    result: function(err, result, fn) { fn(err == null && result == 101); }
  }, {
    name: 'doc3.termlist',
    obj: 'doc3',
    method: 'termlist',
    parameters: [],
    result: function(err, result, fn) {
      fn(err == null && result.length == 1 && result[0].tname == 'a');
    }
  }, {
    name: 'doc3.add_boolean_term',
    obj: 'doc3',
    method: 'add_boolean_term',
    parameters: [ "b" ],
    result: function(err, result, fn) { fn(err == null); }
  }, {
    name: 'db.add_document(doc3)',
    fatal: true,
    action: function(objects, sync, fn) { 
      if (sync) {
        objects.db.add_document(objects.doc3);
        fn(null); 
      } else {
        objects.db.add_document(objects.doc3, function(err, result) {
          fn(err);
        });
      }
    }
  }, {
    name: 'db.get_doclength',
    obj: 'db',
    method: 'get_doclength',
    parameters: [ 4 ],
    result: function(err, result, fn) { fn(err == null && result == 101); }
  }, {
    name: 'doc3.remove_term',
    obj: 'doc3',
    method: 'remove_term',
    parameters: [ "a" ],
    result: function(err, result, fn) { fn(err == null); }
  }, {
    name: 'db.add_document(doc3)',
    fatal: true,
    action: function(objects, sync, fn) { 
      if (sync) {
        objects.db.add_document(objects.doc3);
        fn(null); 
      } else {
        objects.db.add_document(objects.doc3, function(err, result) {
          fn(err);
        });
      }
    }
  }, {
    name: 'db.get_doclength',
    obj: 'db',
    method: 'get_doclength',
    parameters: [ 5 ],
    result: function(err, result, fn) { fn(err == null && result == 0); }
  }, {
    name: 'db.close',
    obj: 'db',
    method: 'close',
    parameters: [],
    result: function(err, result, fn) { fn(err == null); }
  }
]
