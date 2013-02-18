#!/usr/bin/env python2
# -*- coding:utf-8 -*-

import re

re0 = re.compile(r'\((\d+)\)')
re1 = re.compile(r'\[?([^\]]+)\]?')

with open('haarcascade_frontalface_alt.m') as f:
    for line in f.readlines():
        print ' '.join(
            (re0.findall(line) +
            re1.findall(line.split('=')[1].strip()[:-2]))
        )

