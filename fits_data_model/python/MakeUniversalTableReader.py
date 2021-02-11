#!/usr/bin/env python3

# Processes the automatically generated header file to produce a C_<StructName>_<Colname> enum entries and an
# UniversalTable reader function.
#
# See vector columns below; may need manual update.
#
# Usage:
# python3 /local/gkovacs/cheops/devel/snippets/MakeUniTableReader.py SCI_RAW_ImageMetadata.hxx ...
# TODO: replace push_back with emplace_back for in-place object construction

import sys

def processOneFile(fname):
    enumL=[] # The enum constants
    tempVecsDef=[] # The temporary vector definitions to store the values
    readerCmds=[] # The reader commands
    columnCmds=[] # The TableColumn constructor commands

    # Type upcasting rules
    tdict= dict( (('uint8_t','uint64_t'),('uint16_t','uint64_t'),('uint32_t','uint64_t'),
              ('int8_t','int64_t'),('int16_t','int64_t'),('int32_t','int64_t'),('float','double'),
          ))

    
    with open(fname,'r') as F:
        for L in F:
            S=L.strip()

            # Identify the line:
            # ---
            # class SciRawImagemetadata : public FitsDalTable

            if 'public FitsDalTable' in S:
                Sname= S.split()[1]
                enumL.append("// {}".format(Sname))
                if Sname == 'SciRawImagemetadata':
                    tempVecsDef.append("constexpr const size_t bin_size = 60;")
                continue
            
            # Identify the getter methods
            # ---
            #          uint8_t         getCellIntegrity() const     {return m_cellIntegrity;}
            #   const uint32_t *       getCellAcquisitionId() const  {return m_cellAcquisitionId;}

            
            if 'getCell' not in S: continue

            S=S.split('getCell')
            pre=S[0].split()
            post=S[1].split('()')

            # Creating an enum name
            enName="C_{}_{}".format(Sname,post[0])
            enumL.append(enName)

            # Creating a temp vec name
            tVecName = "v_{}".format(post[0])

            if pre[-1]=='*':
                newtype=tdict.get(pre[-2],pre[-2]) # Upcasting
                tempVecsDef.append("std::vector<std::vector<{}>> {} {{}};".format(newtype,tVecName))
                readerCmds.append("{}.push_back(makeVectorCell<{}>(dataptr->getCell{}(),bin_size));".format(tVecName,newtype,post[0]))
            else:            
                if pre[-1]=='&':
                    newtype = pre[-2]
                else:
                    newtype = pre[-1]
                newtype=tdict.get(newtype,newtype) # Upcasting
                tempVecsDef.append("std::vector<{}> {} {{}};".format(newtype,tVecName))
                readerCmds.append(
                    "{}.push_back(dataptr->getCell{}());".format(tVecName,post[0]))

            columnCmds.append("vec_cols.emplace_back(QLP::{},std::move({}));".format(enName,tVecName))
    readerFunc="\
UniversalTable::t_uptr UniversalTable::read{0}({0} * const dataptr)\n\
{{\n\
    if (dataptr == nullptr) return t_uptr();\n\
    \n\
    {1}\n\
    while (dataptr->ReadRow())\n\
    {{\n\
      {2}\n\
    }};\n\
\n\
    t_vecOfColumns vec_cols {{}};\n\
    {3}\n\
    return t_uptr(new UniversalTable(std::move(vec_cols)));\n\
}};\n\
".format(Sname,"\n".join(tempVecsDef),"\n".join(readerCmds),"\n".join(columnCmds))
    return readerFunc, enumL


def processMultiFiles(fnamelist):
    enums=[]
    print ("Writing QLTableReaders.cxx")
    with open('QLTableReaders.cxx','w') as RF:
        print (
"""/** ****************************************************************************
 *  @file
 *  @ingroup ql_algorithms
 *
 *  @brief THIS IS AN AUTOMATICALLY GENERATED FILE. DO NOT EDIT MANUALLY. UniversalTable reader methods.
 *
 *  @author Gabor Kovacs UCA
 */

// first include system header files

// second include CHEOPS common header files

#include "QLTable.hxx"
#include "QLTableColumns.hxx"

""",file=RF)
        for fname in fnamelist:
            readerFunc, enumL = processOneFile(fname)
            enums.extend(enumL)

            print (readerFunc,file=RF);
            print ("/* ***************** */\n",file=RF)

    print ("Writing QLTableColumns.hxx")
    with open('QLTableColumns.hxx','w') as CE:
        print("""
/** ****************************************************************************
 *  @file
 *  @ingroup algorithms
 *
 *  @brief THIS IS AN AUTOMATICALLY GENERATED FILE. DO NOT EDIT MANUALLY. UniversalTable reader methods.
 *
 *  @author Gabor Kovacs
 */

#ifndef _QL_TABLECOLUMNS_HXX_
#define _QL_TABLECOLUMNS_HXX_

""",file=CE)
        print ("namespace QLP {{\n\
\n\
enum columns_en:int\n\
{{\n\
{}\n\
}};\n\
\n\
}};\n\
".format(",\n".join(enums)),file=CE)
        print ("""
#endif /* _QL_TABLECOLUMNS_HXX_ */
""",file=CE)

# Usage: file name.hxx
if __name__ == "__main__":
    processMultiFiles(sys.argv[1:])
