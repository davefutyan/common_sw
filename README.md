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
Once the above third party software has been installed, the common_sw package can be installed as follows:

* cd common_sw
* make install
