var lXapian = require('../xapian-binding');

exports.name = "Query basic tests";
exports.tests = [
  {
    name: 'q1 = new Query',
    fatal: true,
    action: function(objects, sync, fn) { objects.q1 = new lXapian.Query({op: lXapian.Query.OP_AND, queries: ["foo", "bar"]}); fn(null); }
  }, {
    name: 'q1.get_description',
    obj: 'q1',
    method: 'get_description',
    parameters: [],
    result: function(err, result, fn) { fn(err == null && result == 'Xapian::Query((foo AND bar))'); }
  }, {
    name: 's1 = q1.serialise',
    fatal: true,
    action: function(objects, sync, fn) { objects.s1 = objects.q1.serialise(); fn(null); }
  }, {
    name: 'q2 = unserialise(s1)',
    fatal: true,
    action: function(objects, sync, fn) { objects.q2 = objects.q1.unserialise(objects.s1); fn(null); }
  }, {
    name: 'q2 = unserialise(s1)',
    fatal: false,
    action: function(objects, sync, fn) { fn(objects.q2.get_description()=='Xapian::Query((foo AND bar))' ? null : 'failed'); }
  }, {
    name: 'q3 = new Query',
    fatal: true,
    action: function(objects, sync, fn) { objects.q3 = new lXapian.Query({op: lXapian.Query.OP_OR, queries: [{tname: "one", pos: 1}, {op: lXapian.Query.OP_AND, queries: ["two", "three"]}]}); fn(null); }
  }, {
    name: 'q3.get_description',
    obj: 'q3',
    method: 'get_description',
    parameters: [],
    result: function(err, result, fn) { fn(err == null && result == 'Xapian::Query((one:(pos=1) OR (two AND three)))'); }
  }, {
    name: 'q4 = new Query',
    fatal: true,
    action: function(objects, sync, fn) { objects.q4 = new lXapian.Query( {op: lXapian.Query.OP_VALUE_RANGE, slot: 1, begin: 'a', end: 'b'}); fn(null); }
  }, {
    name: 'q4.get_description',
    obj: 'q4',
    method: 'get_description',
    parameters: [],
    result: function(err, result, fn) {fn(err == null && result == 'Xapian::Query(VALUE_RANGE 1 a b)'); }
  }, {
    name: 'q5 = new Query',
    fatal: true,
    action: function(objects, sync, fn) { objects.q5 = new lXapian.Query( {op: lXapian.Query.OP_VALUE_GE, slot: 1, value: 'a'}); fn(null); }
  }, {
    name: 'q5.get_description',
    obj: 'q5',
    method: 'get_description',
    parameters: [],
    result: function(err, result, fn) { fn(err == null && result == 'Xapian::Query(VALUE_GE 1 a)'); }
  }, {
    name: 'q6 = new Query',
    fatal: true,
    action: function(objects, sync, fn) { objects.q6 = new lXapian.Query({op: lXapian.Query.OP_AND, left: "foo", right: "bar"}); fn(null); }
  }, {
    name: 'q6.get_description',
    obj: 'q6',
    method: 'get_description',
    parameters: [],
    result: function(err, result, fn) { fn(err == null && result == 'Xapian::Query((foo AND bar))'); }
  }, {
    name: 'q7 = new Query',
    fatal: true,
    action: function(objects, sync, fn) { objects.q7 = new lXapian.Query({op: lXapian.Query.OP_SCALE_WEIGHT, query: 'foo', parameter: 0.5}); fn(null); }
  }, {
    name: 'q7.get_description',
    obj: 'q7',
    method: 'get_description',
    parameters: [],
    result: function(err, result, fn) { fn(err == null && result == 'Xapian::Query(0.5 * foo)'); }
  }
]

/*

 OP_SCALE_WEIGHT            {op: string, query: QueryObject, parameter: number} 

*/

