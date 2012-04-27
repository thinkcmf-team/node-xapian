```
Compactor
  TODO

  
Database
  methods similar to the C++ API
    Database()
        Create a Database with no databases in. 
    Database(string_path)
        Open a Database, automatically determining the database backend to use. 
    add_database_sync(Database)
        Add an existing database (or group of databases) to those accessed by this object. 
    reopen_sync()
        Re-open the database. 
    close_sync()
        Close the database.
    get_description_sync() - return string
        Return a string describing this object. 
    has_positions_sync() - return bool
        Does this database have any positional information? 
    get_doccount_sync() - return uint32
        Get the number of documents in the database. 
    get_lastdocid_sync() - return uint32
        Get the highest document id which has been used in the database. 
    get_avlength_sync() - return number
        Get the average length of the documents in the database. 
    get_termfreq_sync(string_tname) - return uint32
        Get the number of documents in the database indexed by a given term. 
    term_exists_sync(string_tname) - return bool
        Check if a given term exists in the database. 
    get_collection_freq_sync(string_tname) - return uint32
        Return the total number of occurrences of the given term. 
    get_value_freq_sync(uint32_slot) - return uint32
        Return the frequency of a given value slot. 
    get_value_lower_bound_sync(uint32_slot) - return string
        Get a lower bound on the values stored in the given value slot. 
    get_value_upper_bound_sync(uint32_slot) - return string
        Get an upper bound on the values stored in the given value slot. 
    get_doclength_lower_bound_sync() - return uint32
        Get a lower bound on the length of a document in this DB. 
    get_doclength_upper_bound_sync() - return uint32
        Get an upper bound on the length of a document in this DB. 
    get_wdf_upper_bound_sync(string_term) - return uint32
        Get an upper bound on the wdf of term term. 
    get_doclength_sync(uint32_did) - return uint32
        Get the length of a document. 
    keep_alive_sync()
        Send a "keep-alive" to remote databases to stop them timing out. 
    get_document_sync(uint32_did) - return object_document
        Get a document from the database, given its document id. 
    get_spelling_suggestion_sync (string_word, uint32_max_edit_distance=2) - return string
        Suggest a spelling correction.
    get_metadata_sync(string_key) - return string
        Get the user-specified metadata associated with a given key. 
    get_uuid_sync() - return string
        Get a UUID for the database. 
  methods different from the C++ API:
    postlist_sync(string_tname) - return an array of { docid: uint32, doclength: uint32, wdf: uint32, description: string } (PostingIterator)
        An array of the postlists for a given term.
    termlist_sync(uint32_did) - return an array of { tname: string, wdf: uint32, termfreq: uint32, description: string } (TermIterator)
        An array of the  terms for a given document. 
    positionlist_sync(uint32_did, string_tname) - return an array of { description: string } (PositionIterator)
        An array of the positions for a given term in a given document.
    allterms_sync() - return an array of { tname: string, wdf: uint32, termfreq: uint32, description: string } (TermIterator)
        An array of all the terms in the database. 
    allterms_sync(string_prefix) - return an array of { tname: string, wdf: uint32, termfreq: uint32, description: string } (TermIterator)
        An array of all the terms with a given prefix in the database. 
    valuestream_sync(uint32_slot) - return an array of { value: string, docid: uint32, valueno: uint32, description: string } (ValueIterator)
        An array of the values in slot slot for each document. 
    spellings_sync() - return an array of { tname: string, wdf: uint32, termfreq: uint32, description: string } (TermIterator)
        An array of all the spelling correction targets. 
    synonyms_sync() - return an array of { tname: string, wdf: uint32, termfreq: uint32, description: string } (TermIterator)
        An array of all the synonyms for a given term. 
    synonym_keys_sync(string_prefix='') - return an array of { tname: string, wdf: uint32, termfreq: uint32, description: string } (TermIterator)
        An array of all terms which have synonyms. 
    metadata_keys_sync(string_prefix='') - return an array of { tname: string, wdf: uint32, termfreq: uint32, description: string } (TermIterator)
        An array of all user-specified metadata keys. 


WritableDatabase - all the methods from Database plus the following
  methods similar to the C++ API
    WritableDatabase()
        Create an empty WritableDatabase.
    WritableDatabase(string_path,uint32_action)
        Open a database for update, automatically determining the database backend to use.
          ?uint32_action can be: DB_CREATE_OR_OPEN, DB_CREATE, DB_CREATE_OR_OVERWRITE, DB_OPEN
          maybe define it as a string?
    commit_sync()
        Commit any pending modifications made to the database. 
    flush_sync()
        Pre-1.1.0 name for commit(). 
    begin_transaction_sync(bool_flushed=true)
        Begin a transaction. 
    commit_transaction_sync()
        Complete the transaction currently in progress. 
    cancel_transaction_sync()
        Abort the transaction currently in progress, discarding the pending modifications made to the database. 
    add_document_sync(object_document) - return uint32
        Add a new document to the database. 
    delete_document_sync(uint32_did)
        Delete a document from the database. 
    delete_document_sync(string_unique_term)
        Delete any documents indexed by a term from the database. 
    replace_document_sync(uint32_did, object_document)
        Replace a given document in the database. 
    replace_document_sync(string_unique_term, object_document) - return uint32_docid
        Replace any documents matching a term. 
    add_spelling_sync(string_word, uint32_freqinc=1)
        Add a word to the spelling dictionary. 
    remove_spelling_sync(string_word, uint32_freqdec=1)
        Remove a word from the spelling dictionary. 
    add_synonym_sync(string_term, string_synonym)
        Add a synonym for a term. 
    remove_synonym_sync(string_erm, string_synonym)
        Remove a synonym for a term. 
    clear_synonyms_sync(string_term)
        Remove all synonyms for a term. 
    set_metadata_sync(string_key, string_value)
        Set the user-specified metadata associated with a given key. 
    get_description_sync() - return string
        Return a string describing this object.


Document
  methods similar to the C++ API
    Document()
        Make a new empty Document. 
    get_value_sync(uint32_slot) - return string
        Get value by number.
    add_value_sync(uint32_slot, string_value)
        Add a new value. 
    remove_value_sync(uint32_slot)
        Remove any value with the given number. 
    clear_values_sync()
        Remove all values associated with the document. 
    get_data_sync() - return string
        Get data stored in the document. 
    set_data_sync(string_data)
        Set data stored in the document. 
    add_posting_sync(string_tname, uint32_tpos, uint32_wdfinc=1)
        Add an occurrence of a term at a particular position. 
    add_term_sync(string_tname, uint32_wdfinc=1)
        Add a term to the document, without positional information. 
    add_boolean_term_sync(string_term)
        Add a boolean filter term to the document. 
    remove_posting_sync(string_tname, uint32_tpos, uint32_wdfdec=1)
        Remove a posting of a term from the document. 
    remove_term_sync(string_tname)
        Remove a term and all postings associated with it. 
    clear_terms_sync()
        Remove all terms (and postings) from the document. 
    termlist_count_sync() - return uint32
        The length of the termlist - i.e. 
    values_count_sync() - return uint32
        Count the values in this document. 
    get_docid_sync() - return uint32
        Get the document id which is associated with this document (if any). 
    serialise_sync() - return string
        Serialise document into a string. 
    get_description_sync() - return string
        Return a string describing this object. 
    unserialise_sync(string_s) - return object_document
        Unserialise a document from a string produced by serialise().
  methods different from the C++ API:
    termlist_sync() - return an array of { tname: string, wdf: uint32, termfreq: uint32, description: string } (TermIterator)
        An array of the terms in this document. 
    values_sync() - return an array of { value: string, docid: uint32, valueno: uint32, description: string } (ValueIterator)
        An array of the values in this document. 


Enquire
  methods similar to the C++ API
    Enquire (object_database)
        Create a Xapian::Enquire object. 
    set_query_sync(object_query, uint32_qlen=0)
        Set the query to run. 
    get_query_sync() - return object_query
        Get the query which has been set. 
    set_collapse_key_sync(uint32_collapse_key, uint32_collapse_max=1)
        Set the collapse key to use for queries. 
    set_docid_order_sync(uint32_order)
        Set the direction in which documents are ordered by document id in the returned MSet. 
        uint32_order can be ASCENDING, DESCENDING ot DONT_CARE
    set_cutoff_sync(int32_percent_cutoff, number_weight_cutoff=0)
        Set the percentage and/or weight cutoffs. 
    set_sort_by_relevance_sync()
        Set the sorting to be by relevance only. 
    set_sort_by_value_sync(uint32_sort_key, bool_reverse)
        Set the sorting to be by value only. 
    set_sort_by_value_then_relevance_sync(uint32_sort_key, bool_reverse)
        Set the sorting to be by value, then by relevance for documents with the same value. 
    set_sort_by_relevance_then_value_sync(uint32_sort_key, bool_reverse)
        Set the sorting to be by relevance then value. 
    get_description_sync() - return string
        Return a string describing this object.     
  methods different from the C++ API:
    get_eset (uint32_maxitems, object_omrset, int32_flags=0, number_k=1.0) - return an array of { term: string, weight: number, description: string } (ESet)
        Get the expand set for the given rset. 
        flags: TODO
    get_matching_terms_sync (uint32_did) - return an array of { tname: string, wdf: uint32, termfreq: uint32, description: string } (TermIterator)
        Get terms which match a given document, by document id. 
    get_mset_sync(uint32_first, uint32_maxitems, uint32_checkatleast=0, object_omrset=null) - return an array of { document: object_document, id: uint32, rank: uint32,  collapse_count: uint32, weight: number, collapse_key: string, description: string, percent: int32 } (MSet)
        Get (a portion of) the match set for the current query. 


RSet
  TODO


Query
  TODO
```