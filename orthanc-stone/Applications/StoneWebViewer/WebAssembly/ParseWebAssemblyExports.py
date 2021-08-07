#!/usr/bin/env python

# Stone of Orthanc
# Copyright (C) 2012-2016 Sebastien Jodogne, Medical Physics
# Department, University Hospital of Liege, Belgium
# Copyright (C) 2017-2021 Osimis S.A., Belgium
#
# This program is free software: you can redistribute it and/or
# modify it under the terms of the GNU Affero General Public License
# as published by the Free Software Foundation, either version 3 of
# the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Affero General Public License for more details.
#
# You should have received a copy of the GNU Affero General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.


# Ubuntu 20.04:
# sudo apt-get install python-clang-6.0
# ./ParseWebAssemblyExports.py --libclang=libclang-6.0.so.1 ./Test.cpp

# Ubuntu 18.04:
# sudo apt-get install python-clang-4.0
# ./ParseWebAssemblyExports.py --libclang=libclang-4.0.so.1 ./Test.cpp

# Ubuntu 14.04:
# ./ParseWebAssemblyExports.py --libclang=libclang-3.6.so.1 ./Test.cpp


import sys
import clang.cindex
import pystache
import argparse

##
## Parse the command-line arguments
##

parser = argparse.ArgumentParser(description = 'Parse WebAssembly C++ source file, and create a basic JavaScript wrapper.')
parser.add_argument('--libclang',
                    default = '',
                    help = 'manually provides the path to the libclang shared library')
parser.add_argument('source', 
                    help = 'Input C++ file')

args = parser.parse_args()



if len(args.libclang) != 0:
    clang.cindex.Config.set_library_file(args.libclang)

index = clang.cindex.Index.create()

# PARSE_SKIP_FUNCTION_BODIES prevents clang from failing because of
# undefined types, which prevents compilation of functions
tu = index.parse(args.source,
                 [ '-DEMSCRIPTEN_KEEPALIVE=__attribute__((annotate("WebAssembly")))',
                   '-DSTONE_WEB_VIEWER_EXPORT=__attribute__((annotate("WebAssembly")))'],
                 options = clang.cindex.TranslationUnit.PARSE_SKIP_FUNCTION_BODIES)



TEMPLATE = '''
const Stone = function() {
  {{#enumerations}}
  this.{{name}} = {
    {{#values}}
    {{name}}: {{value}}{{separator}}
    {{/values}}
  };

  {{/enumerations}}
  {{#functions}}
  this._{{name}} = undefined;
  {{/functions}}
};

Stone.prototype.Setup = function(Module) {
  {{#functions}}
  this._{{name}} = Module.cwrap('{{name}}', {{{returnType}}}, [ {{#args}}{{{type}}}{{^last}}, {{/last}}{{/args}} ]);
  {{/functions}}
};

{{#functions}}
Stone.prototype.{{name}} = function({{#args}}{{name}}{{^last}}, {{/last}}{{/args}}) {
  {{#hasReturn}}return {{/hasReturn}}this._{{name}}({{#args}}{{name}}{{^last}}, {{/last}}{{/args}});
};

{{/functions}}
var stone = new Stone();
'''




# WARNING: Undefined types are mapped as "int"

functions = []
enumerations = []

def ToUpperCase(source):
    target = source[0]
    for c in source[1:]:
        if c.isupper():
            target += '_'
        target += c.upper()
    return target
    


def IsExported(node):
    for child in node.get_children():
        if (child.kind == clang.cindex.CursorKind.ANNOTATE_ATTR and
            child.displayname == 'WebAssembly'):
            return True

    return False


def Explore(node):
    if node.kind == clang.cindex.CursorKind.ENUM_DECL:
        if IsExported(node):
            name = node.spelling
            values = []
            for value in node.get_children():
                if (value.spelling.startswith(name + '_')):
                    s = value.spelling[len(name) + 1:]
                    if len(values) > 0:
                        values[-1]['separator'] = ','
                    values.append({
                        'name' : ToUpperCase(s),
                        'value' : value.enum_value
                    })
                    
            enumerations.append({
                'name' : name,
                'values' : values
                })
    
    if node.kind == clang.cindex.CursorKind.FUNCTION_DECL:
        if IsExported(node):
            f = {
                'name' : node.spelling,
                'args' : [],
            }

            returnType = node.result_type.spelling
            if returnType == 'void':
                f['hasReturn'] = False
                f['returnType'] = "null"
            elif returnType == 'const char *':
                f['hasReturn'] = True
                f['returnType'] = "'string'"
            elif returnType in [ 'int', 'unsigned int' ]:
                f['hasReturn'] = True
                f['returnType'] = "'int'"
            else:
                raise Exception('Unknown return type in function "%s()": %s' % (node.spelling, returnType))

            for child in node.get_children():
                if child.kind == clang.cindex.CursorKind.PARM_DECL:
                    arg = {
                        'name' : child.displayname,
                    }
                    
                    argType = child.type.spelling
                    if argType == 'int':
                        arg['type'] = "'int'"
                    elif argType == 'const char *':
                        arg['type'] = "'string'"
                    else:
                        raise Exception('Unknown type for argument "%s" in function "%s()": %s' %
                                        (child.displayname, node.spelling, argType))

                    f['args'].append(arg)

            if len(f['args']) != 0:
                f['args'][-1]['last'] = True
                    
            functions.append(f)

    for child in node.get_children():
        Explore(child)

Explore(tu.cursor)



print(pystache.render(TEMPLATE, {
    'functions' : functions,
    'enumerations' : enumerations
}))
