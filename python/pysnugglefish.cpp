#include <Python.h>
#include <structmember.h>
#include <stdio.h>
#include <string>
#include <vector>
#include "../include/snugglefish.h"

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

/*
* Facilitate destruction of pysnugglefish objects.
*/
static void pysnugglefish_dealloc(pysnugglefish* self) {
	Py_XDECREF(self->index);
	Py_XDECREF(self->file_list);
    self->ob_type->tp_free((PyObject*)self);
}

/*
* Construct a new pysnugglefish object.
*/
static PyObject * pysnugglefish_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
    pysnugglefish *self;
    self = (pysnugglefish *)type->tp_alloc(type, 0);
    if (self != NULL) { // if object created, init fields to defaults
        self->index = PyString_FromString("");
        if (self->index == NULL) {
            Py_DECREF(self);
            return NULL;
         }
        self->file_list = PyString_FromString("");
        if (self->file_list == NULL) {
            Py_DECREF(self);
            return NULL;
         }
        self->max_buffer = 0;
        self->ngram_size = 3;
        self->max_files = 0;
    }

    return (PyObject *)self;
}

/*
* Initialize a pysnugglefish object from command line arguments.
*/
static int pysnugglefish_init(pysnugglefish *self, PyObject *args, PyObject *kwds) {
    PyObject *index=NULL, *tmp;
	int ngrams = 3;

    static char *kwlist[] = {"index", "ngram_size", NULL};

    if (! PyArg_ParseTupleAndKeywords(args, kwds, "|Si", kwlist, &index, &ngrams)) {
        return -1; // index path and ngram size optional
	}

	if (ngrams != 2 && ngrams != 3) {
	    PyErr_SetString(PyExc_TypeError, "N-Gram size must be set to 2 or 3.");
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

/*
* Define members of pysnugglefish object.
*/
static PyMemberDef pysnugglefish_members[] = {
    {"index", T_OBJECT_EX, offsetof(pysnugglefish, index), 0,
     "index name"},
    {"file_list", T_OBJECT_EX, offsetof(pysnugglefish, file_list), 0,
     "file names"},
    {"ngram_size", T_INT, offsetof(pysnugglefish, ngram_size), 0,
     "n-gram size"},
    {"max_buffer", T_INT, offsetof(pysnugglefish, max_buffer), 0,
     "max buffer size"},
    {"max_files", T_INT, offsetof(pysnugglefish, max_files), 0,
     "max files to use"},
    { NULL }  /* Sentinel */
};

/*
* Getter defined for index member.
*/
static PyObject * pysnugglefish_getindex(pysnugglefish *self, void *closure) {
    Py_INCREF(self->index);
    return self->index;
}

/*
* Setter defined for index member.
*/
static int pysnugglefish_setindex(pysnugglefish *self, PyObject *value, void *closure) {
  if (value == NULL) {
    PyErr_SetString(PyExc_TypeError, "Cannot delete the index attribute");
    return -1;
  }
  
  if (! PyString_Check(value)) {
    PyErr_SetString(PyExc_TypeError, "The index attribute value must be a string");
    return -1;
  }
      
  Py_DECREF(self->index);
  Py_INCREF(value);
  self->index = value;    

  return 0;
}

/*
* Getter defined for file_list member.
*/
static PyObject * pysnugglefish_getfilelist(pysnugglefish *self, void *closure) {
    Py_INCREF(self->file_list);
    return self->file_list;
}

/*
* Setter defined for index member.
*/
static int pysnugglefish_setfilelist(pysnugglefish *self, PyObject *value, void *closure) {
  if (value == NULL) {
    PyErr_SetString(PyExc_TypeError, "Cannot delete the file_list attribute");
    return -1;
  }
  
  if (! PyString_Check(value)) {
    PyErr_SetString(PyExc_TypeError, 
                    "The file_list attribute value must be a string");
    return -1;
  }
      
  Py_DECREF(self->file_list);
  Py_INCREF(value);
  self->file_list = value;    

  return 0;
}

/*
* Define set of getters/setters for reference by Python module.
*/
static PyGetSetDef pysnugglefish_getseters[] = {
    {"index", 
     (getter)pysnugglefish_getindex, (setter)pysnugglefish_setindex,
     "index",
     NULL},
    {"file_list", 
     (getter)pysnugglefish_getfilelist, (setter)pysnugglefish_setfilelist,
     "file_list",
     NULL},
    {NULL}  /* Sentinel */
};

/*
* Search a snugglefish index for a given search input.
* Args: searchString (string)
*/
static PyObject * pysnugglefish_search(pysnugglefish * self, PyObject *args) {
    char *searchString;
    if (!PyArg_ParseTuple(args, "s", &searchString)) {
        return NULL;
    }
  
	vector<string> found = search(PyString_AsString(self->index), searchString, self->ngram_size, false);
	
    PyObject *ret = PyList_New(found.size());
    for(int i = 0; i < found.size(); i++) {
        PyList_SetItem(ret, i, Py_BuildValue("s", found[i].c_str()));
	}
    return ret;
}

/*
* Index all of the files specified in the file_list member.
* The file_list attribute must be a semicolon-delimited list.
* Output the index at the path specified in the pysnugglefish index member.
*/
static PyObject * pysnugglefish_index(pysnugglefish * self) {
	vector<string> fileNames;
	char *files = PyString_AsString(self->file_list);
	
	// parse the file_list member by splitting on semicolon
	files = strtok (files, ";");
    while (files != NULL) {
		fileNames.push_back(files);
        files = strtok (NULL, ";");
    }
	
	// pass off to snugglefish
	make_index(PyString_AsString(self->index), fileNames, self->ngram_size, self->max_files, self->max_buffer);
	self->file_list = Py_BuildValue("s", ""); // reset the file_list attribute
	
    Py_INCREF(Py_None);
    return Py_None;
}

/*
* Define the set of methods callable from a pysnugglefish object.
*/
static PyMethodDef pysnugglefish_methods[] = {
	{"search", (PyCFunction)pysnugglefish_search, METH_VARARGS, "Search the current index for an input."},
	{"make_index", (PyCFunction)pysnugglefish_index, METH_NOARGS, "Make an index out of the current files list."},
    { NULL }  /* Sentinel */
};

/*
* Define the Python type for pysnugglefish objects.
* Configures pysnugglefish with its getters, setters, destructors, etc.
*/
PyTypeObject pysnugglefish_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                         			/*ob_size*/
    "pysnugglefish",             		/*tp_name*/
    sizeof(pysnugglefish),             	/*tp_basicsize*/
    0,                         			/*tp_itemsize*/
    (destructor)pysnugglefish_dealloc, 	/*tp_dealloc*/
    0,                         			/*tp_print*/
    0,                         			/*tp_getattr*/
    0,                         			/*tp_setattr*/
    0,                         			/*tp_compare*/
    0,                         			/*tp_repr*/
    0,                         			/*tp_as_number*/
    0,                         			/*tp_as_sequence*/
    0,                         			/*tp_as_mapping*/
    0,                         			/*tp_hash */
    0,                         			/*tp_call*/
    0,                         			/*tp_str*/
    0,                         			/*tp_getattro*/
    0,                         			/*tp_setattro*/
    0,                         			/*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
    "pysnugglefish objects",           	/* tp_doc */
    0,		               				/* tp_traverse */
    0,		               				/* tp_clear */
    0,		               				/* tp_richcompare */
    0,		               				/* tp_weaklistoffset */
    0,		               				/* tp_iter */
    0,		               				/* tp_iternext */
    pysnugglefish_methods,             	/* tp_methods */
    pysnugglefish_members,             	/* tp_members */
    pysnugglefish_getseters,           	/* tp_getset */
    0,                         			/* tp_base */
    0,                         			/* tp_dict */
    0,                         			/* tp_descr_get */
    0,                         			/* tp_descr_set */
    0,			                        /* tp_dictoffset */
    (initproc)pysnugglefish_init,      	/* tp_init */
    0,                         			/* tp_alloc */
    pysnugglefish_new,                 	/* tp_new */
};

/*
* Define static module methods (none).
*/
static PyMethodDef module_methods[] = {
    {NULL}  /* Sentinel */
};

/*
* Initialize the module.
* Ensures that the pysnugglefish type is ready, makes methods available to objects,
* and prepares error object for any future issues with pysnugglefish.
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
    
    SnuggleError = PyErr_NewException("pysnugglefish.error", NULL, NULL);
    Py_INCREF(SnuggleError);
    PyModule_AddObject(m, "error", SnuggleError);
    Py_INCREF(&pysnugglefish_Type);
    PyModule_AddObject(m, "init", (PyObject *)&pysnugglefish_Type);
}