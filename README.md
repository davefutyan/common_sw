# common_sw
Software framework for [CHEOPS](https://cheops.unibe.ch) science operations, developed by Reiner Rohlfs, Anja Bekkelien and Mathias Beck at the University of Geneva, required by [CHEOPSim](https://github.com/davefutyan/CHEOPSim).

<h3>Requirements</h3>
The CHEOPS software framework can be installed on linux or macOS. The CHEOPS deployment uses CentOs version 7.5.

<br>gcc 5.4.0 is required to compile the software: https://ftp.gnu.org/gnu/gcc/gcc-5.4.0/
<br>The gcc installation in turn requires the following to be installed first:

* GMP 6.1.2 or later: https://gmplib.org/
* MPFR Library 4.0.1 or later: http://www.mpfr.org/
* MPC Library 1.1.0 or later: http://www.multiprecision.org/

Before compiling common_sw, the following external packages need to be compiled and installed using gcc 5.4.0:

* boost version 1.59.0 w/python3.6.5 support: http://sourceforge.net/projects/boost/files/boost/1.59.0/
* cfitsio version 3.45 or later: https://heasarc.gsfc.nasa.gov/fitsio/
* xerces-c version 3.2.2 or later: http://xerces.apache.org/xerces-c/
* xsd version 4.0.0: http://www.codesynthesis.com/products/xsd/

Python 3.6.5 or later is also required

<h3>Environment</h3>
The environment variable CHEOPS_SW needs to be defined by the user to specify a path for the installation, and the PATH, PYTHONPATH, and LD_LIBRARY_PATH environment variables need to be updated as follows:

    export PYTHONPATH=${CHEOPS_SW}/python:${PYTHONPATH}
    export PATH=${CHEOPS_SW}/bin:$PATH
    export LD_LIBRARY_PATH=${CHEOPS_SW}/lib:${LD_LIBRARY_PATH}
    
Paths to the binaries for python, xerces-c and xsd must also be added to PATH
<br>Paths to the libraries for python, boost, cfitsio and xerces-c must also be added to LD_LIBRARY_PATH

In addition, the following environment variables need to be set:
    
    export EXT_INC_DIRS="-I${BOOST_INC_PATH} -I${PYTHON_INC_PATH} -I${CFITSIO_INC_PATH}"
    export EXT_LIB_DIRS="-Wl,-rpath,${BOOST_LIB_PATH} -L${BOOST_LIB_PATH} -L${PYTHON_LIB_PATH} -L${CFITSIO_LIB_PATH}"

where:
 * PYTHON_INC_PATH, BOOST_INC_PATH, CFITSIO_INC_PATH are the paths to the include files for installations of python, boost and cfitsio, respectively.
 * PYTHON_LIB_PATH, BOOST_LIB_PATH, CFITSIO_LIB_PATH are the paths to the shared libraries for installations of python, boost and cfitsio, respectively.

<h3>Installation</h3>
common_sw can now be installed as follows:

    cd common_sw
    make install
