{
  'targets': [
    {
      'target_name': 'xapian-binding',
      'sources': [ "src/binding.cc", "src/database.cc", "src/document.cc", "src/enquire.cc", 
        "src/query.cc", "src/stem.cc", "src/rset.cc", "src/termgenerator.cc", "src/writabledatabase.cc" ],
      'include_dirs': [ '/usr/local/include/xapian-1.3', '/usr/local/lib' ], #FIX: add relative path
      'link_settings': {
        'libraries': [ '-lxapian-1.3' ]
      },
      'cflags_cc': ['-fexceptions']
    }
  ]
}
