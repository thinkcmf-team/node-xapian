For Node v0.4.x

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

Mime2Text surfaces a proposed Xapian patch from
  https://github.com/networkimprov/xapian/commits/liam_mime2text-lib
  that patch is included here as an x86 Linux binary, libmime2text.a

Todo:

Update for Node v0.6

Support more Query constructor variants


Future development:
All sync methods will have an async version with an extra parameter (a callback function)


Database
  methods similar to the C++ API
    Database()
        Create a Database with no databases in. 
    Database(strPath)
        Open a Database, automatically determining the database backend to use. 
    addDatabaseSync(Database)
        Add an existing database (or group of databases) to those accessed by this object. 
    reopenSync()
        Re-open the database. 
    closeSync()
        Close the database.
    getDescriptionSync() - return string
        Return a string describing this object. 
    hasPositionsSync() - return bool
        Does this database have any positional information? 
    getDoccountSync() - return int
        Get the number of documents in the database. 
    getLastdocidSync() - return int
        Get the highest document id which has been used in the database. 
    getAvlengthSync() - return double
        Get the average length of the documents in the database. 
    getTermfreqSync(strTname) - return int
        Get the number of documents in the database indexed by a given term. 
    termExistsSync(strTname) - return bool
        Check if a given term exists in the database. 
    getCollectionFreqSync(strTname) - return int
        Return the total number of occurrences of the given term. 
    getValueFreqSync(iSlot) - return int
        Return the frequency of a given value slot. 
    getValueLowerBoundSync(iSlot) - return string
        Get a lower bound on the values stored in the given value slot. 
    getValueUpperBoundSync(iSlot) - return string
        Get an upper bound on the values stored in the given value slot. 
    getDoclengthLowerBoundSync() - return int
        Get a lower bound on the length of a document in this DB. 
    getDoclengthUpperBoundSync() - return int
        Get an upper bound on the length of a document in this DB. 
    getWdfUpperBoundSync(strTerm) - return int
        Get an upper bound on the wdf of term term. 
    getDoclengthSync(iDid) - return int
        Get the length of a document. 
    keepAliveSync()
        Send a "keep-alive" to remote databases to stop them timing out. 
    getDocumentSync(iDid) - return Document
        Get a document from the database, given its document id. 
    getSpellingSuggestionSync (strWord, iMaxEditDistance=2) - return string
        Suggest a spelling correction.
    getMetadataSync(strKey) - return string
        Get the user-specified metadata associated with a given key. 
    getUuidSync() - return string
        Get a UUID for the database. 
  methods different from the C++ API:
    postlistSync(strTname) - return an array of { docid: int, doclength: int, wdf: int, description: str } (PostingIterator)
        An array of the postlists for a given term.
    termlistSync(iDid) - return an array of { tname: str, wdf: int, termfreq: int, description: str } (TermIterator)
        An array of the  terms for a given document. 
    positionlistSync(iDid, strTname) - return an array of { description: str } (PositionIterator)
        An array of the positions for a given term in a given document.
    alltermsSync() - return an array of { tname: str, wdf: int, termfreq: int, description: str } (TermIterator)
        An array of all the terms in the database. 
    alltermsSync(strPrefix) - return an array of { tname: str, wdf: int, termfreq: int, description: str } (TermIterator)
        An array of all the terms with a given prefix in the database. 
    valuestreamSync(iSlot) - return an array of { value: str, docid: int, valueno: int, description: str } (ValueIterator)
        An array of the values in slot slot for each document. 
    spellingsSync() - return an array of { tname: str, wdf: int, termfreq: int, description: str } (TermIterator)
        An array of all the spelling correction targets. 
    synonymsSync() - return an array of { tname: str, wdf: int, termfreq: int, description: str } (TermIterator)
        An array of all the synonyms for a given term. 
    synonymKeysSync(strPrefix='') - return an array of { tname: str, wdf: int, termfreq: int, description: str } (TermIterator)
        An array of all terms which have synonyms. 
    metadataKeysSync(strPrefix='') - return an array of { tname: str, wdf: int, termfreq: int, description: str } (TermIterator)
        An array of all user-specified metadata keys. 