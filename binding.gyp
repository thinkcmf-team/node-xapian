{
  'targets': [
    {
      'target_name': 'xapian-binding',
      'sources': [ "src/binding.cc", "src/database.cc", "src/document.cc", "src/enquire.cc", 
        "src/query.cc", "src/stem.cc", "src/rset.cc", "src/termgenerator.cc", "src/writabledatabase.cc" ],
      'link_settings': {
        'libraries': [ '<!(xapian-config --libs)', '../libmime2text.a' ]
      },
      'cflags_cc': [ '<!(xapian-config --cxxflags)', '-fexceptions', "-g", "-D_FILE_OFFSET_BITS=64", "-D_LARGEFILE_SOURCE", "-Wall"] #FIX: check to see which flags to keep
    }
  ]
}
