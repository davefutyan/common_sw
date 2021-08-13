# common_sw
CHEOPS software framework required by CHEOPSim, developed by Reiner Rohlfs, Anja Bekkelien and Mathias Beck at the University of Geneva

<h3>Requirements</h3>
The CHEOPS software framework can be compiled and run on linux or macOS. The CHEOPS deployment uses CentOs version 7.5.

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

<h3>Installation</h3>
The environment variable CHEOPS_SW needs to be defined by the user to specify the path of the installation.

<br>The following additional environment variables need to be set:

    export CHEOPS_TESTDATA=${CHEOPS_SW}
    export PYTHONPATH=${CHEOPS_SW}/python:${CHEOPS_SW}/lib:${PYTHONPATH}
    export EXT_INC_DIRS="-I${BOOST_PATH}/include -I${PYTHON_PATH}/include -I${CFITSIO_PATH}/include"
    export EXT_LIB_DIRS="-Wl,-rpath,${BOOST_PATH}/lib -L${BOOST_PATH}/lib -L${PYTHON_PATH}/lib -L${CFITSIO_PATH}/lib"
    export PATH=${PYTHON_PATH}/bin:${XERCES_PATH}/bin:${CHEOPS_SW}/bin:${PATH}
    export LD_LIBRARY_PATH=${PYTHON_PATH}/lib:${BOOST_PATH}/lib:${CFITSIO_PATH}/lib:${XERCES_PATH}/lib:${CHEOPS_SW}/lib:${LD_LIBRARY_PATH}

where PYTHON_PATH, BOOST_PATH, CFITSIO_PATH and XERCES_PATH are the paths to the installations of python, boost, cfitsio and xerces, respectively.

common_sw can now be installed as follows:

    cd common_sw
    make install
