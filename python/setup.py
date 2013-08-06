from distutils.core import setup, Extension

INCLUDE_DIRS  = ['/usr/local/include', '/opt/local/include', '/usr/include', '../include', '../src']
LIBRARY_DIRS  = ['/usr/lib', '/usr/local/lib']

# the c++ extension module
extension_mod = Extension("pysnugglefish",
                          sources=["pysnugglefish.c"],
                          include_dirs = INCLUDE_DIRS,
                          library_dirs = LIBRARY_DIRS
                          )


setup (# Distribution meta-data
       name = "pysnugglefish",
       version = "0.1",
       description = "python bindings for snugglefish",
       author = "Shayne Snow",
       author_email = "scaswell@mitre.org",
       license = "BSD",
       long_description = "Python bindings for snugglefish",
       ext_modules = [ extension_mod ]
       )