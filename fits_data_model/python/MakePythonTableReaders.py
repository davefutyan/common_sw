#!/usr/bin/env python3


# Usage:
# Run in the common_sw/fits_data_model/src folder where the PyExt_xxx.cxx files already exist
# python3 ../../../snippets/MakePythonTableReaders.py "'SCI_RAW_HkDefault', 'SCI_RAW_HkExtended', 'SCI_RAW_HkIfsw', ... "

import sys
import re

code_format_template="\
    @staticmethod\n\
    def read{product}(fname= None, S= None, masked= False):\n\
        \"\"\"!Loads the given {product} data structure using the boost.python interface and packs the data into \n\
         an astropy Table\n\
    \n\
        THIS IS A SEMI-AUTOMATICALLY GENERATED METHOD, DO NOT EDIT MANUALLY.\n\
        @param fname the path to a fits table extension.\n\
        @param S fits_data_model object holding a fits table extension.\n\
        @param masked if True, the returned table is a masked table. Checks for NULL values and\n\
         sets mask where NULL values occurr. Only columns with corresponding\n\
         isNull function are checked, all other values remain unmasked (mask = False).\n\
        @return Table data, meta = OrderedDict of fits header info. Table is None if the fits file has no rows as column \n\
        types cannot be determined in this case.\n\
        \"\"\"\n\
    \n\
        if S is None:\n\
            S = fits_data_model.{product}(fname)\n\
    \n\
        names = [{keynames}]\n\
    \n\
        getters = ({keygetters})\n\
    \n\
        rows = []\n\
        for getFunc in getters:\n\
            rows.append(getFunc(S))\n\
    \n\
        HDR = OrderedDict(zip(names, rows))\n\
    \n\
        # Table data\n\
    \n\
        col_names = ({colnames})\n\
    \n\
        getters = ({colgetters})\n\
    \n\
        cols = [list() for x in range(len(col_names))]\n\
    \n\
        if masked:\n\
            # Process rows and produce mask for NULL values\n\
     \n\
            getmasks = ({colgetmasks})\n\
     \n\
            areThereMasks = ({istherenull})\n\
     \n\
            masks= [list() for x in range(len(getmasks))]\n\
            processcells = tuple(zip(cols, getters))\n\
            processmasks = tuple(zip(masks,getmasks))\n\
            while S.readRow():\n\
                for C, getFunc in processcells:\n\
                    C.append(getFunc(S))\n\
                for M, getMaskFunc in processmasks:\n\
                    M.append(getMaskFunc(S))\n\
    \n\
            if len(cols[0]) < 1:\n\
                DAT = None\n\
            else:\n\
                DAT = Table(cols, names=col_names, masked=True)\n\
                fullmasks=[]\n\
                masks.reverse()\n\
                for C in areThereMasks:\n\
                    if C:\n\
                        fullmasks.append(masks.pop())\n\
                    else:\n\
                        fullmasks.append(False)\n\
                DAT.mask = fullmasks\n\
                DAT.meta = HDR\n\
    \n\
        else:\n\
            # With no masks\n\
    \n\
            processcells = tuple(zip(cols, getters))\n\
            while S.readRow():\n\
                for C, getFunc in processcells:\n\
                    C.append(getFunc(S))\n\
            if len(cols[0]) < 1:\n\
                DAT = None\n\
            else:\n\
                DAT = Table(cols, names=col_names, masked=False)\n\
                DAT.meta = HDR\n\
    \n\
        return DAT\n\
"

readAllTableBeginTemplate= "\
    def readAllTables(self):\n\
        \"\"\"!\n\
        @brief Reads all supported standalone (not associated to any other data product being in the same file) \n\
        data tables from the input fits files into astropy tables.\n\
        \n\
        @details Iterates through the data structure names received in the job order file and reads the standalone binary tables.\n\
        Sets `m_allInputs`, `m_allInputs_hdr`, `m_inputfirsts`, `m_inputlasts`.\n\
        \n\
        @note THIS IS A SEMI-AUTOMATICALLY GENERATED METHOD. \n\
        CONSIDER RUNNING THE CODE GENERATOR SCRIPT MakePythonTableReaders.py BEFORE CHANGING ANYTHING.\n\
        \"\"\"\n\
        # === BEGIN \n\
        structNames = self.m_params.getInputStructNames()\n\
        pass_ids = []  # Checking pass id-s\n\
        self.m_inputfirsts = [\n\
            0, ]  # [first,last) indices for data structures in allInputs\n\
        self.m_allInputs=[]\n\
        self.m_allInputs_hdr=[]\n\
        self.m_allFnames=[]\n\
"

readAllTableLoopTemplate="\
        # === \n\
\n\
        l_dat = []  # List of data tables\n\
        l_hdr = []\n\
        l_fnames = []\n\
\n\
        if '{dname}' in structNames:\n\
\n\
            inputFiles = self.m_params.getInputFiles('{dname}')\n\
\n\
            logger.debug(\n\
                \"Loading {{0}} {dname} structures\".format(len(inputFiles)))\n\
\n\
            for fname in inputFiles:\n\
                bname = os.path.basename(\n\
                    fname)  # bname still contains the structure name at the end\n\
\n\
                dat, hdr = self.read{product}(fname)  # Read in all files\n\
\n\
                if dat is None:\n\
                    logger.warning(\n\
                        '{{0}} is an empty structure. Skipping.'.format(bname))\n\
                else:\n\
                    pass_ids.append(hdr['PassId'])\n\
                    l_hdr.append(hdr)\n\
                    l_dat.append(dat)\n\
                    l_fnames.append(bname)\n\
\n\
        self.m_allInputs.extend(l_dat)\n\
        self.m_allInputs_hdr.extend(l_hdr)\n\
        self.m_allFnames.extend(l_fnames)\n\
        self.m_inputfirsts.append(self.m_inputfirsts[-1] + len(l_dat))\n\
\n\
        self.m_all{product} = l_dat\n\
        self.m_all{product}_hdr = l_hdr\n\
        self.m_all{product}_filenames = l_fnames\n\
\n\
"
readAllTableEndTemplate="\
        # === END \n\
\n\
        self.m_inputlasts = self.m_inputfirsts[1:]\n\
        self.m_pass_ids = pass_ids\n\
"

unittestTemplate="\
    def testGet{product}(self): \n\
        self.removeFile(self.IOfname)\n\
        S = fits_data_model.{product}(self.IOfname, \"CREATE\")\n\
        if hasattr(S, \"setKeyPassId\"):\n\
            S.setKeyPassId(fits_data_model.PassId())\n\
        if hasattr(S, \"setKeyVisitId\"):\n\
            S.setKeyVisitId(fits_data_model.VisitId(10, 0, 0, 1))\n\
        S.writeRow()\n\
        S.writeRow()\n\
        del S\n\
        \n\
        DAT = fits_data_model.IOHandler.IOHandler.read{product}(self.IOfname)\n\
        \n\
        self.assertIsInstance(DAT, astropy.table.Table)\n\
        self.assertEqual(len(DAT), 2) # We have written 2 rows\n\
        \n\
        # Empty case\n\
        \n\
        self.removeFile(self.IOfname)\n\
        S = fits_data_model.{product}(self.IOfname, \"CREATE\")\n\
        if hasattr(S, \"setKeyPassId\"):\n\
            S.setKeyPassId(fits_data_model.PassId())\n\
        if hasattr(S, \"setKeyVisitId\"):\n\
            S.setKeyVisitId(fits_data_model.VisitId(10, 0, 0, 1))\n\
        del S\n\
        \n\
        DAT = fits_data_model.IOHandler.IOHandler.read{product}(self.IOfname)\n\
        self.assertIsNone(DAT)\n\
\n\
"


def processOneFile(fname):
    
    with open(fname,'r') as F:
        D = dict()
        keyL=['',]
        keygetterL=['',]
        colL=['',]
        colgetterL=['',]
        getmasksL=['',]
        isThereNull=['',]
        
        colname = ""
        lastnull = None

        exp_dname = re.compile(r'#include\s+"PyExt_(\w+)\.hxx.*')
        exp_class=re.compile(r'class_<\w+>\("(\w+)",.*\)')
        exp_key=re.compile(r'\.def\("getKey(\w+)",.*\)')
        exp_col=re.compile(r'\.def\("getCell(\w+)",.*\)')
        exp_isnull=re.compile(r'\.def\("isNull(\w+)",.*\)')
   
        for L in F:
            S=L.strip()

             #Identify the line:
             #---
             # .def("getKeyExtVer", &SciRawHkdefaultWrapper::getKeyExtVer)
             
            M=exp_dname.match(S)
            if M:
                D['dname']=M.group(1)
                
            M=exp_class.match(S)
            if M:
                D['product']=M.group(1)
            
            M=exp_key.match(S)
            if M:
                n=M.group(1)
                keyL.append('         "{}",'.format(n))
                keygetterL.append('         fits_data_model.{}.getKey{},'.format(D['product'], n))
            
            M=exp_isnull.match(S)
            if M:
                # This must follow the getCell entry before new column
                n=M.group(1)
                assert n == colname, "isNull entry must follow the getCell entry before new cell name"
                getmasksL.append('             fits_data_model.{}.isNull{},'.format(D['product'], n))
                lastnull = True

            M=exp_col.match(S)
            if M:
                if lastnull is not None:
                    if lastnull:
                        isThereNull.append('             True,')
                    else:
                        isThereNull.append('             False,')

                colname=M.group(1)
                colL.append('         "{}",'.format(colname))
                colgetterL.append('         fits_data_model.{}.getCell{},'.format(D['product'], colname))
                lastnull = False

        if lastnull is not None:
            if lastnull:
                isThereNull.append('             True,')
            else:
                isThereNull.append('             False,')

        D['keynames']='\n'.join(keyL)
        D['keygetters']='\n'.join(keygetterL)
        D['colnames']='\n'.join(colL)
        D['colgetters']='\n'.join(colgetterL)
        D['colgetmasks']='\n'.join(getmasksL)
        D['istherenull']='\n'.join(isThereNull)
                
    return code_format_template.format(**D), readAllTableLoopTemplate.format(**D),  unittestTemplate.format(**D)
   

def processMultiFiles(fnamelist):
    print("Writing functions into python_readers.py")
    print("Writing functions into python_readAll.py")
    print("Writing unit test functions into python_reader_unittest.py")
    RF = open("python_readers.py", "w")
    RA = open("python_readAll.py",  "w")
    UT = open("python_reader_unittest.py",  "w")
    print (readAllTableBeginTemplate.format(), file=RA)
    for fname in fnamelist:
        readerFunc,  readAll,  unitTest = processOneFile(fname)        
        print (readerFunc,file=RF);
        print ("# *****************\n",file=RF)
        print (readAll,file=RA);
        print ("# *****************\n",file=RA)
        print (unitTest,file=UT);
        print ("# *****************\n",file=UT)
    print (readAllTableEndTemplate.format(), file=RA)

def getFilenames():
    fnames=[]
    L=sys.argv[1].split(',')
    for N in L:
        N=N.strip() # possible spaces
        N=N[1:-1] # quotes
        fnames.append("PyExt_{}.cxx".format(N))
    return fnames

# Usage: file name.hxx
if __name__ == "__main__":
    fnames=getFilenames()
    processMultiFiles(fnames)
