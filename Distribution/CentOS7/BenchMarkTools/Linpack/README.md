# Linpack

LINPACK是线性系统软件包(Linear system package) 的缩写。

Linpack现在在国际上已经成为最流行的用于测试高性能计算机系统浮点性能的benchmark。通过利用高性能计算机，用高斯消元法求解N元一次稠密线性代数方程组的测试，评价高性能计算机的浮点性能。

- HPL : A Portable Implementation of the High-Performance Linpack Benchmark for Distributed-Memory Computers. (http://www.netlib.org/benchmark/hpl/hpl-2.1.tar.gz)

- BLAS : The BLAS (Basic Linear Algebra Subprograms) are routines that provide standard building blocks for performing basic vector and matrix operations. (http://www.netlib.org/blas/blas.tgz)

- CBLAS : Reference implementation for the C interface to the Legacy BLAS. (http://www.netlib.org/blas/blast-forum/cblas.tgz)


## 测试环境
- CPU: ft200plus 64core / ARMv8
- OS: CentOS7 for aarch64 
- Mpich + BLAS + hpl


## BLAS编译&安装
```
#> yum install gcc-gfortran
#> make
#> cp blas_LINUX.a /lib64/
```

## CBLAS编译&安装
```
#> vim Makefile.in
BLLIB = /lib64/blas_LINUX.a

#> make
#> cp lib/cblas_LINUX.a /lib64/
```

## hpl编译
```
#> yum install mvapich2-2.0-devel libibmad-static.aarch64

#> cat Make.Linux_aarch64
SHELL        = /bin/sh
CD           = cd
CP           = cp
LN_S         = ln -s
MKDIR        = mkdir
RM           = /bin/rm -f
TOUCH        = touch
ARCH         = Linux_aarch64
TOPdir       = /opt/hpl-2.1
INCdir       = $(TOPdir)/include
BINdir       = $(TOPdir)/bin/$(ARCH)
LIBdir       = $(TOPdir)/lib/$(ARCH)
HPLlib       = $(LIBdir)/libhpl.a
MPdir        = /usr/lib64/mvapich2
MPinc        = -I$(MPdir)/include/mvapich2-aarch64
MPlib        = $(MPdir)/lib/libmpich.a
LAdir        = /lib64
LAinc        =
LAlib        = $(LAdir)/cblas_LINUX.a $(LAdir)/blas_LINUX.a
F2CDEFS      =
HPL_INCLUDES = -I$(INCdir) -I$(INCdir)/$(ARCH) $(LAinc) $(MPinc)
HPL_LIBS     = $(HPLlib) $(LAlib) $(MPlib)
HPL_OPTS     = -DHPL_CALL_CBLAS
HPL_DEFS     = $(F2CDEFS) $(HPL_OPTS) $(HPL_INCLUDES)
CC           = /usr/lib64/mvapich2/bin/mpicc
CCNOOPT      = $(HPL_DEFS)
CCFLAGS      = $(HPL_DEFS) -fomit-frame-pointer -O3 -funroll-loops
LINKER       = /usr/lib64/mvapich2/bin/mpif77
LINKFLAGS    = $(CCFLAGS)
ARCHIVER     = ar
ARFLAGS      = r
RANLIB       = echo

#> make arch=Linux_aarch64

#> ll bin/Linux_aarch64/*
bin/Linux_aarch64/HPL.dat
bin/Linux_aarch64/xhpl
```

## 运行测试命令
```
#> pwd
/opt/hpl-2.1/bin/Linux_aarch64
#> /usr/lib64/mvapich2/bin/mpirun -np 64 ./xhpl
```

## 测试结果 
```
================================================================================
HPLinpack 2.1  --  High-Performance Linpack benchmark  --   October 26, 2012
Written by A. Petitet and R. Clint Whaley,  Innovative Computing Laboratory, UTK
Modified by Piotr Luszczek, Innovative Computing Laboratory, UTK
Modified by Julien Langou, University of Colorado Denver
================================================================================

An explanation of the input/output parameters follows:
T/V    : Wall time / encoded variant.
N      : The order of the coefficient matrix A.
NB     : The partitioning blocking factor.
P      : The number of process rows.
Q      : The number of process columns.
Time   : Time in seconds to solve the linear system.
Gflops : Rate of execution for solving the linear system.

The following parameter values will be used:

N      :      29       30       34       35
NB     :       1        2        3        4
PMAP   : Row-major process mapping
P      :       2        1        4
Q      :       2        4        1
PFACT  :    Left    Crout    Right
NBMIN  :       2        4
NDIV   :       2
RFACT  :    Left    Crout    Right
BCAST  :   1ring
DEPTH  :       0
SWAP   : Mix (threshold = 64)
L1     : transposed form
U      : transposed form
EQUIL  : yes
ALIGN  : 8 double precision words

--------------------------------------------------------------------------------

- The matrix A is randomly generated for each test.
- The following scaled residual check will be computed:
      ||Ax-b||_oo / ( eps * ( || x ||_oo * || A ||_oo + || b ||_oo ) * N )
- The relative machine precision (eps) is taken to be               1.110223e-16
- Computational tests pass if scaled residuals are less than                16.0

================================================================================
T/V                N    NB     P     Q               Time                 Gflops
--------------------------------------------------------------------------------
WR00L2L2          29     1     2     2               0.00              6.857e-03
HPL_pdgesv() start time Thu Jul 16 09:52:45 2020

HPL_pdgesv() end time   Thu Jul 16 09:52:45 2020

--------------------------------------------------------------------------------
||Ax-b||_oo/(eps*(||A||_oo*||x||_oo+||b||_oo)*N)=        0.0188218 ...... PASSED
================================================================================
T/V                N    NB     P     Q               Time                 Gflops
--------------------------------------------------------------------------------
WR00L2L4          29     1     2     2               0.00              7.169e-03
HPL_pdgesv() start time Thu Jul 16 09:52:45 2020

HPL_pdgesv() end time   Thu Jul 16 09:52:45 2020

--------------------------------------------------------------------------------
||Ax-b||_oo/(eps*(||A||_oo*||x||_oo+||b||_oo)*N)=        0.0188218 ...... PASSED
================================================================================
T/V                N    NB     P     Q               Time                 Gflops
--------------------------------------------------------------------------------
WR00L2C2          29     1     2     2               0.00              7.179e-03
HPL_pdgesv() start time Thu Jul 16 09:52:45 2020

HPL_pdgesv() end time   Thu Jul 16 09:52:45 2020

        ......
        ......
        ......

--------------------------------------------------------------------------------
||Ax-b||_oo/(eps*(||A||_oo*||x||_oo+||b||_oo)*N)=        0.0166399 ...... PASSED
================================================================================
T/V                N    NB     P     Q               Time                 Gflops
--------------------------------------------------------------------------------
WR00R2R2          35     4     4     1               0.00              2.637e-02
HPL_pdgesv() start time Thu Jul 16 09:52:47 2020

HPL_pdgesv() end time   Thu Jul 16 09:52:47 2020

--------------------------------------------------------------------------------
||Ax-b||_oo/(eps*(||A||_oo*||x||_oo+||b||_oo)*N)=        0.0166399 ...... PASSED
================================================================================
T/V                N    NB     P     Q               Time                 Gflops
--------------------------------------------------------------------------------
WR00R2R4          35     4     4     1               0.00              2.512e-02
HPL_pdgesv() start time Thu Jul 16 09:52:47 2020

HPL_pdgesv() end time   Thu Jul 16 09:52:47 2020

--------------------------------------------------------------------------------
||Ax-b||_oo/(eps*(||A||_oo*||x||_oo+||b||_oo)*N)=        0.0166399 ...... PASSED
================================================================================

Finished    864 tests with the following results:
            863 tests completed and passed residual checks,
              1 tests completed and failed residual checks,
              0 tests skipped because of illegal input values.
--------------------------------------------------------------------------------

End of Tests.
================================================================================
```