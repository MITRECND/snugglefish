#include <Python.h>

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
    search(indexFileName, searchString, ngramSize);
    
    return Py_BuildValue("");
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
    index(indexFileName, fileNames, ngramSize);
    
    return Py_BuildValue("");
}

static PyMethodDef PySnugglefishMethods[] = {
    { "search", search_wrapper, METH_VARARGS, "Search" },
    { "index", index_wrapper, METH_VARARGS, "Index" },
    { NULL, NULL, 0, NULL }
};

DL_EXPORT(void) initpysnugglefish(void)
{
    Py_InitModule("pysnugglefish", PySnugglefishMethods);
}