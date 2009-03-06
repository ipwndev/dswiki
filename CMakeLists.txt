CMAKE_MINIMUM_REQUIRED ( VERSION 2.6 )
PROJECT ( indexer )
ADD_EXECUTABLE ( indexer indexer.cpp char_convert.cpp )
TARGET_LINK_LIBRARIES( indexer z bz2 )
