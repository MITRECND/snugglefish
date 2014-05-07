PySnugglefish
=======================

Python bindings for Snugglefish

Description
===========
PySnugglefish is a python wrapper around the snugglefish C++ codebase. It exposes the search and index functions to python modules.

PySnugglefish Instances
=======================
The pysnugglefish module is pretty simple to utilize.
Start by importing the module, it's pretty easy:

		import pysnugglefish

To execute snugglefish functions, create an interface through pysnugglefish:

		obj = pysnugglefish.init("/path/to/indexfile")
		

Or, you can specify the index file and the ngram size as init arguments.
		
		ngram_sz = 3
		obj = pysnugglefish.init("/path/to/indexfile", ngram_sz)

Ngram size must be either 3 or 4.

Indexing with PySnugglefish
===========================

To index, simply feed the pysnugglefish instance with configuration options, then run the indexing function. 

		obj = pysnugglefish.init("/path/to/indexfile")
		obj.file_list = ["/path/to/file1", "/path/to/file2"]
		obj.ngram_size = 3 # defaults to 3
		obj.max_buffer = 9001 # defaults to no maximum (0)
		obj.max_files = 100000 # defaults to no maximum (0)
		obj.make_index() # create the index file

If you are indexing lots of files this will be very memory and CPU intensive, so be patient.

Searching with PySnugglefish
============================
The module facilitates searching a specified index.
Again, provide configuration, then execute your search.

		obj = pysnugglefish.init("/path/to/indexfile")
		obj.ngram_size = 3 # better equal the ngram_size used to generate the index!
		bitstring = "\x41\x42\x43"
		files_found = obj.search(bitstring)
		
The search function returns an array containing the filenames of each file in the index which the snugglefish code matched to the input search string.

Caveats
=======
Python doesn't interpret tildes in paths automatically. This pysnugglefish module does not do any special processing on the files provided to the index function. So, expect issues with paths such as ~/Documents/file.bin
