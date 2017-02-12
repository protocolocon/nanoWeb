#  -*- mode: python; coding: utf-8; -*-
#
#    Contributors: Asier Aguirre
#
#    All rights reserved. Use of this source code is governed by a
#    BSD-style license that can be found in the LICENSE.txt file.

import subprocess

def analyze(data, mapping):
    i = data.find('// EMSCRIPTEN_START_FUNCS')
    f = data.find('// EMSCRIPTEN_END_FUNCS')
    if f < 0: f = len(data)
    i = data.find('function ', i, f)

    while i >= 0:
        ii = data.find('function ', i + 1, f)
        if i > 0:
            size = ((ii < 0) and f or ii) - i
            endName = data.find('(', i + 9, f)
            funcName = data[i+9:endName]
            # try to map symbols
            try:
                funcNameMap = mapping and mapping[funcName] or funcName
                funcName = funcNameMap
            except:
                pass
            # try to demangle
            try:
                output = subprocess.check_output(['c++filt', funcName[1:]]).decode().strip()
                funcName = output
            except:
                pass
            print('%9d %9d %s' % (i, size, funcName))
        i = ii

if __name__ == "__main__":
    import sys
    if len(sys.argv) < 2:
        print(sys.argv[0], '<filename>')
    else:
        try:
            mapping = {}
            for line in open(sys.argv[1] + '.symbols').read().split('\n'):
                mapping[line[:line.find(':')]] = line[line.find(':') + 1:]
        except:
            mapping = None
        analyze(open(sys.argv[1]).read(), mapping)
