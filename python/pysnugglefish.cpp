#include <Python.h>
#include <stdio.h>
#include <string>
#include <vector>
#include "../src/fileIndexer.cpp"
#include "../src/nGramBase.cpp"
#include "../src/nGramIndex.cpp"
#include "../src/nGramSearch.cpp"

using namespace std;

static PyObject *SnuggleError;

static PyObject * search_wrapper(PyObject * self, PyObject * args)
{
    char * indexFileName;
    char * searchString;
    int ngramSize;
    
    // parse arguments
    if (!PyArg_ParseTuple(args, "ssi", &indexFileName, &searchString, &ngramSize)) {
        return NULL;
    }
    
    // run the actual search function
    if(strlen(searchString) < ngramSize){
        PyErr_SetString(SnuggleError, "Search string size is smaller than Ngram size, the search string must be greater than or equal to the ngram size");
        return NULL;
    }
    
    nGramSearch searcher(ngramSize, indexFileName);
    vector <string> results = searcher.searchNGrams(searcher.stringToNGrams(searchString));
    
    for(uint32_t i = 0; i < results.size(); i++){
        std::cout << results[i] << std::endl;
        //printf("%s\n", results[i]);
    }
    //search(indexFileName, searchString, ngramSize);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject * index_wrapper(PyObject * self, PyObject * args)
{
    char * indexFileName;
    char * fileNames;
    int ngramSize;
    
    // parse arguments
    if (!PyArg_ParseTuple(args, "ssi", &indexFileName, &fileNames, &ngramSize)) {
        return NULL;
    }
    
    // run the actual indexing function
    //index(indexFileName, fileNames, ngramSize);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyMethodDef PySnugglefishMethods[] = {
    { "search", search_wrapper, METH_VARARGS, "Search an index for an input." },
    { "index", index_wrapper, METH_VARARGS, "Index a set of input files." },
    { NULL, NULL, 0, NULL }
};

PyMODINIT_FUNC initpysnugglefish(void)
{
    PyObject *m;
    
    m = Py_InitModule("pysnugglefish", PySnugglefishMethods);
    if (m == NULL)
        return;
    
    SnuggleError = PyErr_NewException("pysnugglefish.error", NULL, NULL);
    Py_INCREF(SnuggleError);
    PyModule_AddObject(m, "error", SnuggleError);
}