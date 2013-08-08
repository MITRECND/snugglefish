PySnugglefish
=======================

Python bindings for Snugglefish

Description
===========
PySnugglefish is a python wrapper around the snugglefish C++ codebase. It exposes the search and index functions to python modules.

Using PySnugglefish
===================
The pysnugglefish module is pretty simple to utilize.
Start by importing the module, it's pretty easy:

		import pysnugglefish

The module exposes the Snugglefish index function.
Just give the method the name of the index file to create, the files to index, and the n-gram size to use.
You may optionally specify both the maximum file count and maximum buffer size.
		
		index = "index-name"
		files = "list; of; filenames"
		size = 3
		pysnugglefish.index(index, files, size)
		
		buffer_size = 9001
		file_count = 2000
		pysnugglefish.index(index, files, size, buffer_size, file_count)
		
The index function returns no value.  

This module also exposes the ability to search an existing index file.

		index = "index-name"
		search = "search content" # search string must be >= size
		size = 3
		pysnugglefish.search(index, search, size)

Caveats
=======
Python doesn't interpret tildes in paths automatically. This pysnugglefish module does not do any special processing on the files provided to the index function. So, expect issues with paths such as ~/Documents/file.bin