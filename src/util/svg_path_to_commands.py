#  -*- mode: python; coding: utf-8; -*-
#
#    Contributors: Asier Aguirre
#
#    All rights reserved. Use of this source code is governed by a
#    BSD-style license that can be found in the LICENSE.txt file.

class Coord:
    def __init__(self, x, y):
        self.x = x
        self.y = y

    def rel(self, c):
        self.x += float(c[0])
        self.y += float(c[1])

    def abs(self, c):
        self.x = float(c[0])
        self.y = float(c[1])

        
class SvgPath:

    def __init__(self):
        self.out = ''
        self.xscale = self.yscale = 1.0
        self.xtrans = self.ytrans = 0.0

    def setXScale(self, v):
        self.xscale = v

    def setYScale(self, v):
        self.yscale = v

    def setXTranslate(self, v):
        self.xtrans = v

    def setYTranslate(self, v):
        self.ytrans = v

    def transform(self, x, y):
        return (x * self.xscale + self.xtrans, y * self.yscale + self.ytrans)
        
    def coordDump(self, coord, rel, pair):
        if rel:
            return '%.3f, %.3f' % self.transform(coord.x + float(pair[0]), coord.y + float(pair[1]))
        else:
            return '%.3f, %.3f' % self.transform(float(pair[0]), float(pair[1]))

    def dump(self, command, coord, rel, values):
        if not self.out:
            self.out += 'beginPath()'
        self.out += ',\n'
        self.out += command + '('
        self.out += ', '.join([self.coordDump(coord, rel, pair) for pair in zip(values[0::2], values[1::2])])
        self.out += ')'
        if values:
            if rel:
                coord.rel(values[-2:])
            else:
                coord.abs(values[-2:])
        return coord

    def add(self, path):
        path = path.replace(',', ' ')
        cs = path.split() # commands
        coord = Coord(0, 0)
        command = 'error'
        rel = True

        i = 0
        while i < len(cs):
            c = cs[i]
            if c == 'm' or c == 'M':
                rel = c.islower()
                command = c == 'm' and 'l' or 'L'
                coord = self.dump('moveto', coord, rel, cs[i + 1 : i + 3])
                i += 2
            elif c == 'l' or c == 'L':
                rel = c.islower()
                command = c
                coord = self.dump('lineto', coord, rel, cs[i + 1 : i + 3])
                i += 2
            elif c == 'c' or c == 'C':
                rel = c.islower()
                command = c
                coord = self.dump('bezierto', coord, rel, cs[i + 1 : i + 7])
                i += 6
            elif c == 'z' or c == 'Z':
                rel = c.islower()
                command = 'error'
                self.dump('closePath', coord, rel, [])
            else:
                assert False, 'unhandled command: ' + str(c)

            i += 1
            if i < len(cs) and not cs[i].isalpha():
                assert command != 'error', 'no command for values'
                i -= 1
                cs[i] = command
            


if __name__ == "__main__":
    import sys
    if len(sys.argv) < 2:
        print(sys.argv[0], '<svg path> <scale X> <scale Y> <translate X> <translate Y>')
    else:
        svg = SvgPath()
        if len(sys.argv) > 2: svg.setXScale(float(sys.argv[2]))
        if len(sys.argv) > 3: svg.setYScale(float(sys.argv[3]))
        if len(sys.argv) > 4: svg.setXTranslate(float(sys.argv[4]))
        if len(sys.argv) > 5: svg.setYTranslate(float(sys.argv[5]))
        svg.add(sys.argv[1])
        print(svg.out)
