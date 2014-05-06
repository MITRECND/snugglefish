/*
Copyright (c) 2014 The MITRE Corporation. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
SUCH DAMAGE.
*/

#include <Python.h>
#include <structmember.h>
#include <stdio.h>
#include <string>
#include <vector>
#include "nGramSearch.h"
#include "nGramIndex.h"
#include "fileindexer.h"

using namespace std;

/* Module's error object. */
static PyObject *SnuggleError;

/*
 * Define the pysnugglefish object.
 * Represents data used in the snugglefish features.
 */
typedef struct {
	PyObject_HEAD
	PyObject *index;
	PyObject *file_list;
	int ngram_size;
	int max_buffer;
	int max_files;
} pysnugglefish;

/* Facilitate destruction of pysnugglefish objects. */
static void pysnugglefish_dealloc(pysnugglefish* self) {
	Py_XDECREF(self->index);
	Py_XDECREF(self->file_list);
	self->ob_type->tp_free((PyObject*)self);
}

/* Construct a new pysnugglefish object. */
static PyObject * pysnugglefish_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
	pysnugglefish *self;
	self = (pysnugglefish *)type->tp_alloc(type, 0);
	if (self != NULL) { // if object created, init fields to defaults
		self->index = PyString_FromString("");
		if (self->index == NULL) {
			Py_DECREF(self);
			return NULL;
		}
		self->file_list = PyList_New(0);
		self->max_buffer = 0;
		self->ngram_size = 3;
		self->max_files = 0;
	}

	return (PyObject *)self;
}

/* Initialize a pysnugglefish object from command line arguments. */
static int pysnugglefish_init(pysnugglefish *self, PyObject *args, PyObject *kwds) {
	PyObject *index=NULL, *tmp;
	int ngrams = 3;

	static char *kwlist[] = {(char *) "index", (char *) "ngram_size", NULL};

	if (! PyArg_ParseTupleAndKeywords(args, kwds, "S|i", kwlist, &index, &ngrams)) {
		return -1; // ngram size optional
	}

	if (ngrams != 3 && ngrams != 4) {
		PyErr_SetString(PyExc_TypeError, "N-Gram size must be set to 3 or 4.");
		return -1;
	}
	if (index) { // if index was provided, manage references and set
		tmp = self->index;
		Py_INCREF(index);
		self->index = index;
		Py_XDECREF(tmp);
	}
	self->ngram_size = ngrams;

	return 0;
}

/* Define members of pysnugglefish object. */
static PyMemberDef pysnugglefish_members[] = {
	{(char *) "index", T_OBJECT_EX, offsetof(pysnugglefish, index), 0,
	 (char *) "index name"},
	{(char *) "file_list", T_OBJECT_EX, offsetof(pysnugglefish, file_list), 0,
	 (char *) "list of file names"},
	{(char *) "ngram_size", T_INT, offsetof(pysnugglefish, ngram_size), 0,
	 (char *) "n-gram size"},
	{(char *) "max_buffer", T_INT, offsetof(pysnugglefish, max_buffer), 0,
	 (char *) "max buffer size"},
	{(char *) "max_files", T_INT, offsetof(pysnugglefish, max_files), 0,
	 (char *) "max files to use"},
	{ NULL }  /* Sentinel */
};

/*
 * Search a snugglefish index for a given search input.
 * Args: searchString (string)
 */
static PyObject * pysnugglefish_search(pysnugglefish * self, PyObject *args) {
	char *searchString;
	vector<string> *found;
	long procs;

	if (!PyArg_ParseTuple(args, "s", &searchString)) {
		return NULL;
	}

	try {
		//This only works on some *nixes
        // TODO figure out which systems don't support this call
		procs = sysconf(_SC_NPROCESSORS_ONLN);
		if (procs < 1){
			procs = 1;
		}
		snugglefish::nGramSearch searcher(self->ngram_size, PyString_AsString(self->index), (uint32_t) procs);
		vector<uint64_t> *ngrams = searcher.stringToNGrams(searchString);
        found = searcher.searchNGrams(*ngrams);
		//found = search(PyString_AsString(self->index), searchString, self->ngram_size, (uint32_t) procs);
	} catch (exception &e) {
		PyErr_SetString(SnuggleError, e.what());
		return NULL;
	}

	PyObject *ret = PyList_New(found->size());
	for(size_t i = 0; i < found->size(); i++) {
		PyList_SetItem(ret, i, Py_BuildValue("s", (*found)[i].c_str()));
	}
	delete found;
	return ret;
}

/*
 * Index all of the files specified in the file_list member.
 * The file_list attribute must be a semicolon-delimited list.
 * Output the index at the path specified in the pysnugglefish index member.
 */
static PyObject * pysnugglefish_index(pysnugglefish * self) {
	vector<string> files;
	//long procs;
	Py_ssize_t ct = PyList_Size(self->file_list);
	int i;

	//This only works on some *nixes
	// TODO figure out which systems don't support this call
	/*
	 * Indexing is single threaded for now, figure out how to do
	 * this properly later.
	 *
	 * -- WXS
	 */
#if 0
	procs = sysconf(_SC_NPROCESSORS_ONLN);
	if (procs < 1){
		procs = 1;
	}
#endif

	for (i = 0; i < ct; i++) {
		files.push_back(PyString_AsString(PyList_GetItem(self->file_list, i)));
		try {
			snugglefish::nGramIndex ngramindex(self->ngram_size, PyString_AsString(self->index));
			ngramindex.setmaxBufferSize(self->max_buffer);
			snugglefish::fileIndexer indexer(self->ngram_size);
			vector<uint32_t> *processedFile = indexer.processFile(PyString_AsString(PyList_GetItem(self->file_list, i)));
			if (processedFile != 0) {
				ngramindex.addNGrams(processedFile, PyString_AsString(PyList_GetItem(self->file_list, i)));
			}
		} catch (exception &e){
			PyErr_SetString(SnuggleError, e.what());
			return NULL;
		}
	}
	Py_INCREF(Py_None);
	return Py_None;
}

/* Define the set of methods callable from a pysnugglefish object. */
static PyMethodDef pysnugglefish_methods[] = {
	{"search", (PyCFunction)pysnugglefish_search, METH_VARARGS, "Search the current index for an input."},
	{"make_index", (PyCFunction)pysnugglefish_index, METH_NOARGS, "Make an index out of the current files list."},
	{ NULL }  /* Sentinel */
};

/* Define getter for data attributes of pysnugglefish objects. */
static PyObject *pysnugglefish_getattr(pysnugglefish *self, char *attrname) {
	if (strcmp(attrname, "index") == 0) {
		return self->index;
	} else if (strcmp(attrname, "file_list") == 0) {
		Py_INCREF(self->file_list);
		return self->file_list;
	} else if (strcmp(attrname, "max_files") == 0) {
		return Py_BuildValue("i", self->max_files);
	} else if (strcmp(attrname, "max_buffer") == 0) {
		return Py_BuildValue("i", self->max_buffer);
	} else if (strcmp(attrname, "ngram_size") == 0) {
		return Py_BuildValue("i", self->ngram_size);
	} else if (strcmp(attrname, "search") == 0) {
		return PyObject_GenericGetAttr((PyObject *)self, Py_BuildValue("s", attrname));
	} else if (strcmp(attrname, "make_index") == 0) {
		return PyObject_GenericGetAttr((PyObject *)self, Py_BuildValue("s", attrname));
	} else {
		PyErr_SetString(PyExc_AttributeError, attrname);
		return NULL;
	}
}

/* Define setter for data attributes of pysnugglefish objects. */
static int pysnugglefish_setattr(pysnugglefish *self, char *name, PyObject *value) {
	int result = -1;
	if (strcmp(name, "index") == 0) {
		PyErr_SetString(SnuggleError, "Index is read-only after init.");
	} else if (strcmp(name, "file_list") == 0) {
		result = 0;
		if (PyList_Check(value) && value != NULL) {
			Py_XDECREF(self->file_list);
			Py_INCREF(value);
			self->file_list = value;
		} else {
			result = -1;
		}
	} else if (strcmp(name, "max_files") == 0 && value != NULL) {
		int newval = 0;
		if (PyArg_Parse(value, "i", &newval)) {
			if (newval > 0) {
				self->max_files = newval;
				result = 0;
			}
		}
	} else if (strcmp(name, "max_buffer") == 0 && value != NULL) {
		int newval = 0;
		if (PyArg_Parse(value, "i", &newval)) {
			if (newval > 0) {
				self->max_buffer = newval;
				result = 0;
			}
		}
	} else if (strcmp(name, "ngram_size") == 0 && value != NULL) {
		int newval = 0;
		if (PyArg_Parse(value, "i", &newval)) {
			if (newval == 3 || newval == 4) {
				self->ngram_size = newval;
				result = 0;
			}
		}
	} else {
		PyErr_SetString(PyExc_AttributeError, name);
		result = -1;
	}
	return result;
}

/*
 * Define the Python type for pysnugglefish objects.
 * Configures pysnugglefish with its getters, setters, destructors, etc.
 */
PyTypeObject pysnugglefish_Type = {
	PyObject_HEAD_INIT(NULL)
	0,                                        /* ob_size */
	"pysnugglefish",                          /* tp_name */
	sizeof(pysnugglefish),                    /* tp_basicsize */
	0,                                        /* tp_itemsize */
	(destructor)pysnugglefish_dealloc,        /* tp_dealloc */
	0,                                        /* tp_print */
	(getattrfunc)pysnugglefish_getattr,       /* tp_getattr */
	(setattrfunc)pysnugglefish_setattr,       /* tp_setattr */
	0,                                        /* tp_compare */
	0,                                        /* tp_repr */
	0,                                        /* tp_as_number */
	0,                                        /* tp_as_sequence */
	0,                                        /* tp_as_mapping */
	0,                                        /* tp_hash */
	0,                                        /* tp_call */
	0,                                        /* tp_str */
	0,                                        /* tp_getattro */
	0,                                        /* tp_setattro */
	0,                                        /* tp_as_buffer */
	Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /* tp_flags */
	"pysnugglefish objects",                  /* tp_doc */
	0,                                        /* tp_traverse */
	0,                                        /* tp_clear */
	0,                                        /* tp_richcompare */
	0,                                        /* tp_weaklistoffset */
	0,                                        /* tp_iter */
	0,                                        /* tp_iternext */
	pysnugglefish_methods,                    /* tp_methods */
	pysnugglefish_members,                    /* tp_members */
	0,                                        /* tp_getset */
	0,                                        /* tp_base */
	0,                                        /* tp_dict */
	0,                                        /* tp_descr_get */
	0,                                        /* tp_descr_set */
	0,                                        /* tp_dictoffset */
	(initproc)pysnugglefish_init,             /* tp_init */
	0,                                        /* tp_alloc */
	pysnugglefish_new,                        /* tp_new */
};

/* Define static module methods (none). */
static PyMethodDef module_methods[] = {
	{NULL}  /* Sentinel */
};

/*
 * Initialize the module.
 * Ensures that the pysnugglefish type is ready, makes methods available to
 * objects, and prepares error object for any future issues with pysnugglefish.
 */
PyMODINIT_FUNC initpysnugglefish(void) {
	PyObject *m;

	if (PyType_Ready(&pysnugglefish_Type) < 0) {
		return;
	}
	m = Py_InitModule3("pysnugglefish", module_methods, "Module that exposes snugglefish methods.");
	if (m == NULL) {
		return;
	}

	SnuggleError = PyErr_NewException((char *) "pysnugglefish.error", NULL, NULL);
	Py_INCREF(SnuggleError);
	PyModule_AddObject(m, "error", SnuggleError);
	Py_INCREF(&pysnugglefish_Type);
	PyModule_AddObject(m, "init", (PyObject *) &pysnugglefish_Type);
}
