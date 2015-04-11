#!/usr/bin/env python

import sys
import re

def is_number(s):
    try:
        float(s)
        return True
    except ValueError:
        return False

def main():
    pline=""
    flag = False
    process=False

    for tmp in sys.stdin:
        line=tmp.rstrip()

        tmp=line.split("/*")

        line=tmp[0]
        line = line.expandtabs(1)

        if flag and not line.startswith("#define") and line.endswith("\\"):
            pline=pline + line

        if flag and not line.endswith("\\"):
            pline = pline + line
            flag = False
            process=True


        if line.startswith("#define") and not line.endswith("\\"):
            flag=False
            process = True
            pline = line

        if line.startswith("#define") and line.endswith("\\"):
            line=line.rstrip("\\")
            flag = True
            pline = line

        if process:
            process=False

            pline = pline.expandtabs(1)
            pline=re.sub( '\s+', ' ', pline ).strip()

            pline=pline.replace("\\","")

            v=pline.split(" ",2)

#            print v
#            print len(v)

            if len(v) == 2:
                v.append( "0xffffffff")

            cmd = v[1] + "=" + v[2]
#            print cmd
            exec(cmd)

            cmd = "op=" + v[1] 
#            print cmd
            exec(cmd)

            if is_number(op):
                inhex=format(op, '#04X')
                print inhex + " constant " + v[1]
            else:
                print op + " constant " + v[1]





main()
