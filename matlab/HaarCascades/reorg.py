#!/usr/bin/env python
#coding: utf-8
import re
re_brac = re.compile("\((\d+)\)")
with open("haarcascade_frontalface_alt.txt", "rb") as fp:
    for line in fp.readlines():
        line = line.strip()
        res = re_brac.findall(line)
        if res:
            val = line.split("=")[-1].strip()
            if val.startswith('['):
               res.extend(list(val[1:-3].split(" ")))
               print ' '.join(res)
            else:
               res.append(val[:-1])
               print ' '.join(res)

