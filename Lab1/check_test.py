#!/usr/bin/python
import os
import readline

def check():
    rootdir = './Tests_advanced/inputs'
    objdir = './Tests_advanced/expects'
    list = os.listdir(objdir)
    total, right = 0, 0
    for i in range(0, len(list)):
        path = os.path.join(rootdir, list[i])
        expect = os.path.join(objdir, list[i])
        path = path[0:len(path)-6]+'cmm'
        # print(list[i])
        result_str = ''
        cmd = 'Code/parser '+path
        f = os.popen(cmd)
        data = f.readlines()
        f.close()
        with open (expect, 'r') as e:
            total = total+1
            line = e.readline()
            j = 0
            flag = 1
            while line:
                if j == len(data) or data[j] != line:
                    flag = 0
                    break
                line = e.readline()
            right=right+flag
            j = j+1
    print(total, right, right/total)

check()