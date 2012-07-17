var xapian = require('../xapian-binding');
var sys = require('sys');

exports.name = "Document basic tests";
exports.tests = [
  {
    name: 'doc1 = new Document',
    fatal: true,
    action: function(objects, sync, fn) { objects.doc1 = new xapian.Document(); fn(null); }
  }, {
    name: 'doc1.set_data',
    obj: 'doc1',
    method: 'set_data',
    parameters: [ "some data" ],
    result: function(err, result, fn) { fn(err == null); }
  }, {
    name: 'doc1.get_data',
    obj: 'doc1',
    method: 'get_data',
    parameters: [],
    result: function(err, result, fn) { fn(err == null && result == 'some data'); }
  }, {
    name: 'doc1.add_value',
    obj: 'doc1',
    method: 'add_value',
    parameters: [ 1, "value 1" ],
    result: function(err, result, fn) { fn(err == null); }
  }, {
    name: 'doc1.add_value',
    obj: 'doc1',
    method: 'add_value',
    parameters: [ 2, "value 2" ],
    result: function(err, result, fn) { fn(err == null); }
  }, {
    name: 'doc1.get_description',
    obj: 'doc1',
    method: 'get_description',
    parameters: [],
    result: function(err, result, fn) { fn(err == null && result == "Document(Xapian::Document::Internal(data=`some data', values[2]))"); }
  }, {
    name: 'doc1.serialise',
    fatal: false,
    action: function(objects, sync, fn) { 
      if (sync) {
        objects.ser1 = objects.doc1.serialise();
        fn(null); 
      } else {
        objects.doc1.serialise(function(err, result) {
          objects.ser1 = result;
          fn(err);
        });
      }
    }
  }, {
    name: 'doc.unserialise',
    fatal: false,
    action: function(objects, sync, fn) { 
      if (sync) {
        objects.doc2 = objects.doc1.unserialise(objects.ser1);
        fn(null); 
      } else {
        objects.doc1.unserialise(objects.ser1, function(err, result) {
          objects.doc2 = result;
          fn(err);
        });
      }
    }
  }, {
    name: 'doc1.get_value',
    obj: 'doc1',
    method: 'get_value',
    parameters: [ 1 ],
    result: function(err, result, fn) { fn(err == null && result === 'value 1'); }
  }, {
    name: 'doc1.values',
    obj: 'doc1',
    method: 'values',
    parameters: [0, 5],
    result: function(err, result, fn) { fn(err == null && result.length == 2 && result[0].valueno == 1); }
  }, {
    name: 'doc1.remove_value 1',
    obj: 'doc1',
    method: 'remove_value',
    parameters: [ 1 ],
    result: function(err, result, fn) { fn(err == null); }
  }, {
    name: 'doc1.values_count',
    obj: 'doc1',
    method: 'values_count',
    parameters: [],
    result: function(err, result, fn) { fn(err == null && result == 1); }
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
    result: function(err, result, fn) { fn(err == null && result == "value 2"); }
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
    name: 'doc1.add_posting',
    obj: 'doc1',
    method: 'add_posting',
    parameters: ['some posting term', 10],
    result: function(err, result, fn) { fn(err == null); }
  }, {
    name: 'doc1.add_posting',
    obj: 'doc1',
    method: 'add_posting',
    parameters: ['some term', 15, 5],
    result: function(err, result, fn) { fn(err == null); }
  }, {
    name: 'doc1.add_term',
    obj: 'doc1',
    method: 'add_term',
    parameters: ['some term'],
    result: function(err, result, fn) { fn(err == null); }
  }, {
    name: 'doc1.add_term',
    obj: 'doc1',
    method: 'add_term',
    parameters: ['some wdf term', 3],
    result: function(err, result, fn) { fn(err == null); }
  }, {
    name: 'doc1.add_boolean_term',
    obj: 'doc1',
    method: 'add_boolean_term',
    parameters: ['some boolean term'],
    result: function(err, result, fn) { fn(err == null); }
  }, {
    name: 'doc1.termlist',
    obj: 'doc1',
    method: 'termlist',
    parameters: [],
    result: function(err, result, fn) { fn(err == null && result.length == 4 && result[2].wdf == 6); }
  },{
    name: 'doc1.termlist',
    obj: 'doc1',
    method: 'termlist',
    parameters: [15,3],
    result: function(err, result, fn) { fn(err == null && result.length == 0); }
  }, {
    name: 'doc1.remove_posting',
    obj: 'doc1',
    method: 'remove_posting',
    parameters: ['some term', 15, 2],
    result: function(err, result, fn) { fn(err == null); }
  }, {
    name: 'doc1.termlist',
    obj: 'doc1',
    method: 'termlist',
    parameters: [],
    result: function(err, result, fn) { fn(err == null && result.length == 4 && result[2].wdf == 4); }
  }, {
    name: 'doc1.remove_term',
    obj: 'doc1',
    method: 'remove_term',
    parameters: ['some wdf term'],
    result: function(err, result, fn) { fn(err == null); }
  }, {
    name: 'doc1.termlist_count',
    obj: 'doc1',
    method: 'termlist_count',
    parameters: [],
    result: function(err, result, fn) { fn(err == null && result == 3); }
  }, {
    name: 'doc1.clear_terms',
    obj: 'doc1',
    method: 'clear_terms',
    parameters: [],
    result: function(err, result, fn) { fn(err == null); }
  }, {
    name: 'doc1.termlist_count',
    obj: 'doc1',
    method: 'termlist_count',
    parameters: [],
    result: function(err, result, fn) { fn(err == null && result == 0); }
  }, {
    name: 'doc1.get_docid',
    obj: 'doc1',
    method: 'get_docid',
    parameters: [],
    result: function(err, result, fn) { fn(err == null && result == 0); }
  }
]

