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
    action: function(objects, sync, fn) { console.log(objects.q2.get_description()); fn(null); }
  }
]

