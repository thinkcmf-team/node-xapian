var lXapian = require('../xapian-binding');

exports.name = "TermGenerator tests from perl";
exports.tests = [
  {
    name: 'termgen = new TermGenerator',
    fatal: true,
    action: function(objects, sync, fn) { objects.termgen = new lXapian.TermGenerator(); fn(null); }
  }, {
    name: 'doc = new Document',
    fatal: true,
    action: function(objects, sync, fn) { objects.doc = new lXapian.Document(); fn(null); }
  }, {
    name: 'termgen.set_document(doc)',
    fatal: true,
    action: function(objects, sync, fn) { 
      if (sync) {
        objects.termgen.set_document(objects.doc); 
        fn(null); 
      } else {
        objects.termgen.set_document(objects.doc, function(err, result) {
          objects.db = result;
          fn(err);
        });
      }
    }
  }, {
    name: 'termgen.index_text("foo bar baz foo")',
    obj: 'termgen',
    method: 'index_text',
    parameters: [ "foo bar baz foo" ],
    result: function(err, result, fn) { fn(err == null); }
  }, {
    name: 'termgen.index_text_without_positions("baz zoo")',
    obj: 'termgen',
    method: 'index_text_without_positions',
    parameters: [ "baz zoo" ],
    result: function(err, result, fn) { fn(err == null); }
  }, {
    name: 'doc.termlist',
    obj: 'doc',
    method: 'termlist',
    parameters: [],
    result: function(err, result, fn) {
      fn(err == null && result.length == 4
        && result[0].tname == 'bar' && result[0].wdf == 1
        && result[1].tname == 'baz' && result[1].wdf == 2
        && result[2].tname == 'foo' && result[2].wdf == 2
        && result[3].tname == 'zoo' && result[3].wdf == 1);
    }
  }
]
