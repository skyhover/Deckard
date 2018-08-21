#! /usr/bin/env python

import sys

if len(sys.argv) !=5 and len(sys.argv) !=4:
    print >> sys.stderr, "Usage:", sys.argv[0], "<license file> <file> <start delimiter> [end delimiter]"
    sys.exit(1)

print "Adding license IN-PLACE:", sys.argv[1:]


lineprefix = ' '
if len(sys.argv)>=5:  # multi-line comments
   if len(sys.argv[3])==1:
      lineprefix = sys.argv[3] + ' '
   elif len(sys.argv[3])>1:
      lineprefix = ' ' + sys.argv[3][-1] + ' '
else: # single-line comments
   lineprefix = sys.argv[3] + ' '

lc = open(sys.argv[1], 'r')
lines = []
# add the first comment
lines.append(sys.argv[3] + '\n')
# add middle comments
for line in lc:
    lines.append(lineprefix + line)
# add the last comment
if len(sys.argv)>=5:
    lines.append(' ' + sys.argv[4] + '\n')
else:
    lines.append(sys.argv[3] + '\n')
lc.close()

f = open(sys.argv[2], 'r')
flines = f.readlines()
f.close()
for line in flines:
    if "copyright" in line.lower():
        print "Error: file already contain license: ", line,
        print "Skip: ", sys.argv[2]
        sys.exit(1)

f = open(sys.argv[2], 'w')
for line in lines:
    print >> f, line,
for line in flines:
    print >> f, line,

print "Done. See file:", sys.argv[2]

