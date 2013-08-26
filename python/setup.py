#
# Copyright (c) 2013 The MITRE Corporation. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
# 
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
# OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
# OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
# SUCH DAMAGE.
#

from distutils.core import setup, Extension

INCLUDE_DIRS  = ['/usr/local/include', '/opt/local/include', '/usr/include', '../include']
LIBRARY_DIRS  = ['/usr/lib', '/usr/local/lib']

# the c++ extension module
extension_mod = Extension("pysnugglefish",
                          sources=["../src/fileIndexer.cpp", "../src/nGramBase.cpp", "../src/nGramIndex.cpp",
                          "../src/nGramSearch.cpp", "../snugglefish.cpp", "pysnugglefish.cpp"],
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