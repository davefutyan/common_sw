#!/usr/bin/env python3

import os
import sys
import time
 
import fits_data_model

pid = None
hdu = None
hduPath = None

def closeFile():
    
    global hdu

    del hdu
    print(str(pid) + ': Closed ' + str(hduPath))
 
def openFile(id, className, path): 

    global pid
    global hdu
    global hduPath

    if hdu is not None:
        closeFile()
    
    clss = getattr(fits_data_model, className)
    pid = id
    hdu = clss(path)
    hduPath = path
    print(str(pid) + ': Opened ' + ' ' + str(hduPath))
    
    

# Expects the following command line arguments:
# 1: an identifier for this process, used in log output
# 2: the class name of a fits data structure
# 3: the path to a fits data structure of the same class
if __name__ == "__main__":
    
    if len(sys.argv) != 4:
        print('Usage: OpenFitsFile ID className path')
        sys.exit(255)
        
    openFile(sys.argv[1], sys.argv[2], sys.argv[3])
    print(str(pid) + ': Sleep 5s')
    time.sleep(5)
    closeFile()
