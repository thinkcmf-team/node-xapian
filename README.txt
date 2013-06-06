For Node v0.10.x

Mirrors Xapian API closely, except:
  Enquire::get_mset() returns an Array, not an iterator
  assemble_document() takes a document parameters object and returns a Document
  Mime2Text provides file-conversion logic from the omindex indexing utility

Classes
  Database
  WritableDatabase
  TermGenerator
  Stem
  Enquire
  Query
  Document
  Mime2Text
  
Documentation in docs.md

Mime2Text surfaces a proposed Xapian patch from
  https://github.com/networkimprov/xapian/commits/liam_mime2text-lib
  that patch is included here as an x86 Linux binary, libmime2text.a
