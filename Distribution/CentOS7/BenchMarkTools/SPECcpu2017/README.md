# SPECcpu2017

1. install
```
#> yum install gcc gcc-c++ gcc-gfortran -y
#> mount -t iso9660 -o,exec,loop cpu2017-1_0_5.iso /mnt/
[root@jerrydai mnt]# ./install.sh
SPEC CPU2017 Installation

Top of the CPU2017 tree is '/mnt'
Enter the directory you wish to install to (e.g. /usr/cpu2017)
/opt/cpu2017

Installing FROM /mnt
Installing TO /opt/cpu2017

Is this correct? (Please enter 'yes' or 'no')
yes

The following toolset is expected to work on your platform.  If the
automatically installed one does not work, please re-run install.sh and
exclude that toolset using the '-e' switch.

The toolset selected will not affect your benchmark scores.

linux-aarch64                 For 64-bit AArch64 systems running Linux.
                              Built with GCC 4.8.4 (Ubuntu/Linaro
                              4.8.4-2ubuntu1~14.04.1) on an HPE
                              Moonshot running Linaro v14.04.1 LTS.

=================================================================
Attempting to install the linux-aarch64 toolset...

Unpacking CPU2017 base files (44 MB)
Unpacking CPU2017 tools binary files (150.2 MB)
Unpacking 500.perlbench_r benchmark and data files (102 MB)
Unpacking 502.gcc_r benchmark and data files (240.3 MB)
Unpacking 503.bwaves_r benchmark and data files (0.2 MB)
Unpacking 505.mcf_r benchmark and data files (8.5 MB)
Unpacking 507.cactuBSSN_r benchmark and data files (12.5 MB)
Unpacking 508.namd_r benchmark and data files (8.3 MB)
Unpacking 510.parest_r benchmark and data files (25.6 MB)
Unpacking 511.povray_r benchmark and data files (23.3 MB)
Unpacking 519.lbm_r benchmark and data files (4.3 MB)
Unpacking 520.omnetpp_r benchmark and data files (56.6 MB)
Unpacking 521.wrf_r benchmark and data files (217.2 MB)
Unpacking 523.xalancbmk_r benchmark and data files (212 MB)
Unpacking 525.x264_r benchmark and data files (57.9 MB)
Unpacking 526.blender_r benchmark and data files (215.7 MB)
Unpacking 527.cam4_r benchmark and data files (348.6 MB)
Unpacking 531.deepsjeng_r benchmark and data files (0.5 MB)
Unpacking 538.imagick_r benchmark and data files (80.7 MB)
Unpacking 541.leela_r benchmark and data files (3.8 MB)
Unpacking 544.nab_r benchmark and data files (38.6 MB)
Unpacking 548.exchange2_r benchmark and data files (0.1 MB)
Unpacking 549.fotonik3d_r benchmark and data files (5.2 MB)
Unpacking 554.roms_r benchmark and data files (11.4 MB)
Unpacking 557.xz_r benchmark and data files (104.1 MB)
Unpacking 600.perlbench_s benchmark and data files (3.1 MB)
Unpacking 602.gcc_s benchmark and data files (0.9 MB)
Unpacking 603.bwaves_s benchmark and data files (0 MB)
Unpacking 605.mcf_s benchmark and data files (0.1 MB)
Unpacking 607.cactuBSSN_s benchmark and data files (0.1 MB)
Unpacking 619.lbm_s benchmark and data files (30.4 MB)
Unpacking 620.omnetpp_s benchmark and data files (0.1 MB)
Unpacking 621.wrf_s benchmark and data files (0.3 MB)
Unpacking 623.xalancbmk_s benchmark and data files (0.1 MB)
Unpacking 625.x264_s benchmark and data files (0.2 MB)
Unpacking 627.cam4_s benchmark and data files (0.5 MB)
Unpacking 628.pop2_s benchmark and data files (283.8 MB)
Unpacking 631.deepsjeng_s benchmark and data files (0.2 MB)
Unpacking 638.imagick_s benchmark and data files (0.3 MB)
Unpacking 641.leela_s benchmark and data files (0 MB)
Unpacking 644.nab_s benchmark and data files (0.1 MB)
Unpacking 648.exchange2_s benchmark and data files (0 MB)
Unpacking 649.fotonik3d_s benchmark and data files (0.1 MB)
Unpacking 654.roms_s benchmark and data files (1.1 MB)
Unpacking 657.xz_s benchmark and data files (0.2 MB)
Unpacking 996.specrand_fs benchmark and data files (0 MB)
Unpacking 997.specrand_fr benchmark and data files (0 MB)
Unpacking 998.specrand_is benchmark and data files (0 MB)
Unpacking 999.specrand_ir benchmark and data files (6.4 MB)

Checking the integrity of your source tree...

Checksums are all okay.

Unpacking binary tools for linux-aarch64...

Checking the integrity of your binary tools...

Checksums are all okay.

Testing the tools installation (this may take a minute)

........................................................

Installation successful.  Source the shrc or cshrc in
/opt/cpu2017
to set up your environment for the benchmark.
```

2. configure
```
#> cd /opt/cpu2017/
#> source shrc
#> cp config/Example-gcc-linux-aarch64.cfg ./ft2000plus-gcc-linux-aarch64.cfg
#> ulimit -s unlimited
```

3. run
```
#> cd bin
#> ./runcpu -c ../ft2000plus-gcc-linux-aarch64.cfg intrate
#> ./runcpu -c ../ft2000plus-gcc-linux-aarch64.cfg intspeed
#> ./runcpu -c ../ft2000plus-gcc-linux-aarch64.cfg fprate
#> ./runcpu -c ../ft2000plus-gcc-linux-aarch64.cfg fpspeed
```

4. result

- SPECrate2017_int_base(intrate)
```
Running Benchmarks
  Running 500.perlbench_r refrate (ref) base cpu_performance_test-64 (64 copies) [2020-07-17 15:56:19]
  Running 502.gcc_r refrate (ref) base cpu_performance_test-64 (64 copies) [2020-07-17 16:15:28]
  Running 505.mcf_r refrate (ref) base cpu_performance_test-64 (64 copies) [2020-07-17 16:41:44]
  Running 520.omnetpp_r refrate (ref) base cpu_performance_test-64 (64 copies) [2020-07-17 17:22:31]
  Running 523.xalancbmk_r refrate (ref) base cpu_performance_test-64 (64 copies) [2020-07-17 18:03:43]
  Running 525.x264_r refrate (ref) base cpu_performance_test-64 (64 copies) [2020-07-17 18:25:08]
  Running 531.deepsjeng_r refrate (ref) base cpu_performance_test-64 (64 copies) [2020-07-17 18:42:40]
  Running 541.leela_r refrate (ref) base cpu_performance_test-64 (64 copies) [2020-07-17 18:55:27]
  Running 548.exchange2_r refrate (ref) base cpu_performance_test-64 (64 copies) [2020-07-17 19:12:03]
  Running 557.xz_r refrate (ref) base cpu_performance_test-64 (64 copies) [2020-07-17 19:40:12]
  Running 999.specrand_ir refrate (ref) base cpu_performance_test-64 (64 copies) [2020-07-17 20:00:15]
Success: 1x500.perlbench_r 1x502.gcc_r 1x505.mcf_r 1x520.omnetpp_r 1x523.xalancbmk_r 1x525.x264_r 1x531.deepsjeng_r 1x541.leela_r 1x548.exchange2_r 1x557.xz_r 1x999.specrand_ir
Producing Raw Reports
 label: cpu_performance_test-64
  workload: refrate (ref)
   metric: SPECrate2017_int_base
    format: raw -> /opt/cpu2017/result/CPU2017.003.intrate.refrate.rsf
Parsing flags for 500.perlbench_r base: done
Parsing flags for 502.gcc_r base: done
Parsing flags for 505.mcf_r base: done
Parsing flags for 520.omnetpp_r base: done
Parsing flags for 523.xalancbmk_r base: done
Parsing flags for 525.x264_r base: done
Parsing flags for 531.deepsjeng_r base: done
Parsing flags for 541.leela_r base: done
Parsing flags for 548.exchange2_r base: done
Parsing flags for 557.xz_r base: done
Doing flag reduction: done
    format: flags -> /opt/cpu2017/result/CPU2017.003.intrate.refrate.flags.html
    format: cfg -> /opt/cpu2017/result/CPU2017.003.intrate.refrate.cfg, /opt/cpu2017/result/CPU2017.003.intrate.refrate.orig.cfg
    format: CSV -> /opt/cpu2017/result/CPU2017.003.intrate.refrate.csv
    format: PDF -> /opt/cpu2017/result/CPU2017.003.intrate.refrate.pdf
    format: HTML -> /opt/cpu2017/result/CPU2017.003.intrate.refrate.html, /opt/cpu2017/result/invalid.gif
    format: Text -> /opt/cpu2017/result/CPU2017.003.intrate.refrate.txt
The log for this run is in /opt/cpu2017/result/CPU2017.003.log

runcpu finished at 2020-07-17 20:00:38; 15116 total seconds elapsed
```
> 69.2

- SPECspeed2017_int_base(intspeed)

```
Running Benchmarks
  Running 600.perlbench_s refspeed (ref) base cpu_performance_test-64 threads:64 [2020-07-20 08:20:40]
  Running 602.gcc_s refspeed (ref) base cpu_performance_test-64 threads:64 [2020-07-20 08:37:47]
  Running 605.mcf_s refspeed (ref) base cpu_performance_test-64 threads:64 [2020-07-20 09:01:47]
  Running 620.omnetpp_s refspeed (ref) base cpu_performance_test-64 threads:64 [2020-07-20 09:39:06]
  Running 623.xalancbmk_s refspeed (ref) base cpu_performance_test-64 threads:64 [2020-07-20 10:00:46]
  Running 625.x264_s refspeed (ref) base cpu_performance_test-64 threads:64 [2020-07-20 10:14:00]
  Running 631.deepsjeng_s refspeed (ref) base cpu_performance_test-64 threads:64 [2020-07-20 10:28:50]
  Running 641.leela_s refspeed (ref) base cpu_performance_test-64 threads:64 [2020-07-20 10:41:13]
  Running 648.exchange2_s refspeed (ref) base cpu_performance_test-64 threads:64 [2020-07-20 10:57:08]
  Running 657.xz_s refspeed (ref) base cpu_performance_test-64 threads:64 [2020-07-20 11:24:25]
  Running 998.specrand_is refspeed (ref) base cpu_performance_test-64 threads:64 [2020-07-20 11:34:22]
Success: 1x600.perlbench_s 1x602.gcc_s 1x605.mcf_s 1x620.omnetpp_s 1x623.xalancbmk_s 1x625.x264_s 1x631.deepsjeng_s 1x641.leela_s 1x648.exchange2_s 1x657.xz_s 1x998.specrand_is
Producing Raw Reports
 label: cpu_performance_test-64
  workload: refspeed (ref)
   metric: SPECspeed2017_int_base
    format: raw -> /opt/cpu2017/result/CPU2017.004.intspeed.refspeed.rsf
Parsing flags for 600.perlbench_s base: done
Parsing flags for 602.gcc_s base: done
Parsing flags for 605.mcf_s base: done
Parsing flags for 620.omnetpp_s base: done
Parsing flags for 623.xalancbmk_s base: done
Parsing flags for 625.x264_s base: done
Parsing flags for 631.deepsjeng_s base: done
Parsing flags for 641.leela_s base: done
Parsing flags for 648.exchange2_s base: done
Parsing flags for 657.xz_s base: done
Doing flag reduction: done
    format: flags -> /opt/cpu2017/result/CPU2017.004.intspeed.refspeed.flags.html
    format: cfg -> /opt/cpu2017/result/CPU2017.004.intspeed.refspeed.cfg, /opt/cpu2017/result/CPU2017.004.intspeed.refspeed.orig.cfg
    format: CSV -> /opt/cpu2017/result/CPU2017.004.intspeed.refspeed.csv
    format: PDF -> /opt/cpu2017/result/CPU2017.004.intspeed.refspeed.pdf
    format: HTML -> /opt/cpu2017/result/CPU2017.004.intspeed.refspeed.html
    format: Text -> /opt/cpu2017/result/CPU2017.004.intspeed.refspeed.txt
The log for this run is in /opt/cpu2017/result/CPU2017.004.log

runcpu finished at 2020-07-20 11:34:47; 12086 total seconds elapsed
```
> 2.23

- SPECspeed2017_fp_base(fprate)

```
Running Benchmarks
  Running 503.bwaves_r refrate (ref) base cpu_performance_test-64 (64 copies) [2020-07-20 12:49:31]
  Running 507.cactuBSSN_r refrate (ref) base cpu_performance_test-64 (64 copies) [2020-07-20 16:00:42]
  Running 508.namd_r refrate (ref) base cpu_performance_test-64 (64 copies) [2020-07-20 17:46:26]
  Running 510.parest_r refrate (ref) base cpu_performance_test-64 (64 copies) [2020-07-20 17:56:08]
  Running 511.povray_r refrate (ref) base cpu_performance_test-64 (64 copies) [2020-07-20 19:05:40]
  Running 519.lbm_r refrate (ref) base cpu_performance_test-64 (64 copies) [2020-07-20 19:29:41]
  Running 521.wrf_r refrate (ref) base cpu_performance_test-64 (64 copies) [2020-07-20 20:52:12]
  Running 526.blender_r refrate (ref) base cpu_performance_test-64 (64 copies) [2020-07-20 21:58:00]
  Running 527.cam4_r refrate (ref) base cpu_performance_test-64 (64 copies) [2020-07-20 22:13:38]
  Running 538.imagick_r refrate (ref) base cpu_performance_test-64 (64 copies) [2020-07-20 22:46:54]
  Running 544.nab_r refrate (ref) base cpu_performance_test-64 (64 copies) [2020-07-20 23:03:19]
  Running 549.fotonik3d_r refrate (ref) base cpu_performance_test-64 (64 copies) [2020-07-20 23:19:49]
  Running 554.roms_r refrate (ref) base cpu_performance_test-64 (64 copies) [2020-07-21 01:02:01]
  Running 997.specrand_fr refrate (ref) base cpu_performance_test-64 (64 copies) [2020-07-21 02:25:45]
Success: 1x503.bwaves_r 1x507.cactuBSSN_r 1x508.namd_r 1x510.parest_r 1x511.povray_r 1x519.lbm_r 1x521.wrf_r 1x526.blender_r 1x527.cam4_r 1x538.imagick_r 1x544.nab_r 1x549.fotonik3d_r 1x554.roms_r 1x997.specrand_fr
Producing Raw Reports
 label: cpu_performance_test-64
  workload: refrate (ref)
   metric: SPECrate2017_fp_base
    format: raw -> /opt/cpu2017/result/CPU2017.005.fprate.refrate.rsf
Parsing flags for 503.bwaves_r base: done
Parsing flags for 507.cactuBSSN_r base: done
Parsing flags for 508.namd_r base: done
Parsing flags for 510.parest_r base: done
Parsing flags for 511.povray_r base: done
Parsing flags for 519.lbm_r base: done
Parsing flags for 521.wrf_r base: done
Parsing flags for 526.blender_r base: done
Parsing flags for 527.cam4_r base: done
Parsing flags for 538.imagick_r base: done
Parsing flags for 544.nab_r base: done
Parsing flags for 549.fotonik3d_r base: done
Parsing flags for 554.roms_r base: done
Doing flag reduction: done
    format: flags -> /opt/cpu2017/result/CPU2017.005.fprate.refrate.flags.html
    format: cfg -> /opt/cpu2017/result/CPU2017.005.fprate.refrate.cfg, /opt/cpu2017/result/CPU2017.005.fprate.refrate.orig.cfg
    format: CSV -> /opt/cpu2017/result/CPU2017.005.fprate.refrate.csv
    format: PDF -> /opt/cpu2017/result/CPU2017.005.fprate.refrate.pdf
    format: HTML -> /opt/cpu2017/result/CPU2017.005.fprate.refrate.html
    format: Text -> /opt/cpu2017/result/CPU2017.005.fprate.refrate.txt
The log for this run is in /opt/cpu2017/result/CPU2017.005.log

runcpu finished at 2020-07-21 02:26:50; 50211 total seconds elapsed
```
> 50.3

- SPECspeed2017_fp_base(fpspeed)

```
Running Benchmarks
  Running 603.bwaves_s refspeed (ref) base cpu_performance_test-64 threads:64 [2020-07-21 09:15:04]
  Running 607.cactuBSSN_s refspeed (ref) base cpu_performance_test-64 threads:64 [2020-07-21 10:21:39]
  Running 619.lbm_s refspeed (ref) base cpu_performance_test-64 threads:64 [2020-07-21 11:18:23]
  Running 621.wrf_s refspeed (ref) base cpu_performance_test-64 threads:64 [2020-07-21 11:30:22]
  Running 627.cam4_s refspeed (ref) base cpu_performance_test-64 threads:64 [2020-07-21 11:48:12]
  Running 628.pop2_s refspeed (ref) base cpu_performance_test-64 threads:64 [2020-07-21 11:51:51]
  Running 638.imagick_s refspeed (ref) base cpu_performance_test-64 threads:64 [2020-07-21 12:02:21]
  Running 644.nab_s refspeed (ref) base cpu_performance_test-64 threads:64 [2020-07-21 12:07:16]
  Running 649.fotonik3d_s refspeed (ref) base cpu_performance_test-64 threads:64 [2020-07-21 12:10:43]
  Running 654.roms_s refspeed (ref) base cpu_performance_test-64 threads:64 [2020-07-21 12:29:10]
  Running 996.specrand_fs refspeed (ref) base cpu_performance_test-64 threads:64 [2020-07-21 12:50:59]
Success: 1x603.bwaves_s 1x607.cactuBSSN_s 1x619.lbm_s 1x621.wrf_s 1x627.cam4_s 1x628.pop2_s 1x638.imagick_s 1x644.nab_s 1x649.fotonik3d_s 1x654.roms_s 1x996.specrand_fs
Producing Raw Reports
 label: cpu_performance_test-64
  workload: refspeed (ref)
   metric: SPECspeed2017_fp_base
    format: raw -> /opt/cpu2017/result/CPU2017.006.fpspeed.refspeed.rsf
Parsing flags for 603.bwaves_s base: done
Parsing flags for 607.cactuBSSN_s base: done
Parsing flags for 619.lbm_s base: done
Parsing flags for 621.wrf_s base: done
Parsing flags for 627.cam4_s base: done
Parsing flags for 628.pop2_s base: done
Parsing flags for 638.imagick_s base: done
Parsing flags for 644.nab_s base: done
Parsing flags for 649.fotonik3d_s base: done
Parsing flags for 654.roms_s base: done
Doing flag reduction: done
    format: flags -> /opt/cpu2017/result/CPU2017.006.fpspeed.refspeed.flags.html
    format: cfg -> /opt/cpu2017/result/CPU2017.006.fpspeed.refspeed.cfg, /opt/cpu2017/result/CPU2017.006.fpspeed.refspeed.orig.cfg
    format: CSV -> /opt/cpu2017/result/CPU2017.006.fpspeed.refspeed.csv
    format: PDF -> /opt/cpu2017/result/CPU2017.006.fpspeed.refspeed.pdf
    format: HTML -> /opt/cpu2017/result/CPU2017.006.fpspeed.refspeed.html
    format: Text -> /opt/cpu2017/result/CPU2017.006.fpspeed.refspeed.txt
The log for this run is in /opt/cpu2017/result/CPU2017.006.log

runcpu finished at 2020-07-21 12:51:23; 14174 total seconds elapsed
```
> 17.1

ft2000plus-gcc-linux-aarch64.cfg
```
#------------------------------------------------------------------------------
# SPEC CPU2017 config file for: gcc / g++ / gfortran on Linux ARM systems
#------------------------------------------------------------------------------
#
# Usage: (1) Copy this to a new name
#             cd $SPEC/config
#             cp Example-x.cfg myname.cfg
#        (2) Change items that are marked 'EDIT' (search for it)
#
# SPEC tested this config file with:
#    Compiler version(s):    5.3, 6.2, 8.1
#    Operating system(s):    Ubuntu 16.04, CentOS 7.4
#    Hardware:               Cavium ThunderX, HPE Moonshot
#
# If your system differs, this config file might not work.
# You might find a better config file at http://www.spec.org/cpu2017/results
#
# Known Limitations
#     It is possible that you might encounter compile time or run time errors
#     with older versions of GCC (for example, 4.x)
#     Recommendation: Use a newer version of the compiler.
#                     If that is not possible, try reducing the optimization.
#
#
# Compiler issues: Contact your compiler vendor, not SPEC.
# For SPEC help:   http://www.spec.org/cpu2017/Docs/techsupport.html
#------------------------------------------------------------------------------

#--------- Label --------------------------------------------------------------
# Arbitrary string to tag binaries (no spaces allowed)
#                  Two Suggestions: # (1) EDIT this label as you try new ideas.
%define label cpu_performance_test                # (2)      Use a label meaningful to *you*.

#--------- Preprocessor -------------------------------------------------------
%ifndef %{bits}                # EDIT to control 32 or 64 bit compilation.  Or,
%   define  bits        64     #      you can set it on the command line using:
%endif                         #      'runcpu --define bits=nn'

%ifndef %{build_ncpus}         # EDIT to adjust number of simultaneous compiles.
%   define  build_ncpus  64    #      Or, you can set it on the command line:
%endif                         #      'runcpu --define build_ncpus=nn'

# Don't change this part.
%define    os           LINUX
%if %{bits} == 64
%   define model        -mabi=lp64
%elif %{bits} == 32
%   define model        -mabi=ilp32
%else
%   error Please define number of bits - see instructions in config file
%endif
%if %{label} =~ m/ /
%   error Your label "%{label}" contains spaces.  Please try underscores instead.
%endif
%if %{label} !~ m/^[a-zA-Z0-9._-]+$/
%   error Illegal character in label "%{label}".  Please use only alphanumerics, underscore, hyphen, and period.
%endif

#--------- Global Settings ----------------------------------------------------
# For info, see:
#            https://www.spec.org/cpu2017/Docs/config.html#fieldname
#   Example: https://www.spec.org/cpu2017/Docs/config.html#tune

command_add_redirect = 1
flagsurl             = $[top]/config/flags/gcc.xml
ignore_errors        = 1
iterations           = 1
label                = %{label}-%{bits}
line_width           = 1020
log_line_width       = 1020
makeflags            = --jobs=%{build_ncpus}
mean_anyway          = 1
output_format        = txt,html,cfg,pdf,csv
preenv               = 1
reportable           = 0
tune                 = base

#--------- How Many CPUs? -----------------------------------------------------
# Both SPECrate and SPECspeed can test multiple chips / cores / hw threads
#    - For SPECrate,  you set the number of copies.
#    - For SPECspeed, you set the number of threads.
# See: https://www.spec.org/cpu2017/Docs/system-requirements.html#MultipleCPUs
#
#    q. How many should I set?
#    a. Unknown, you will have to try it and see!
#
# To get you started, some suggestions:
#
#     copies - This config file defaults to testing only 1 copy.   You might
#              try changing it to match the number of cores on your system,
#              or perhaps the number of virtual CPUs as reported by:
#                     grep -c processor /proc/cpuinfo
#              Be sure you have enough memory.  See:
#              https://www.spec.org/cpu2017/Docs/system-requirements.html#memory
#
#     threads - This config file sets a starting point.  You could try raising
#               it.  A higher thread count is much more likely to be useful for
#               fpspeed than for intspeed.
#
intrate,fprate:
   copies           = 64   # EDIT to change number of copies (see above)
intspeed,fpspeed:
   threads          = 64   # EDIT to change number of OpenMP threads (see above)

#------- Compilers ------------------------------------------------------------
default:
#  EDIT: The parent directory for your compiler.
#        Do not include the trailing /bin/
#        Do not include a trailing slash
#  Examples:
#   1  On a Red Hat system, you said:
#      'yum install devtoolset-6'
#      Use:                 %   define gcc_dir /opt/rh/devtoolset-6/root/usr
#
#   2  You built GCC in:                       /disk1/mybuild/gcc-8.1.0/bin/gcc
#      Use:                 %   define gcc_dir /disk1/mybuild/gcc-8.1.0
#
#   3  You want:                               /usr/bin/gcc
#      Use:                 %   define gcc_dir /usr
#      WARNING: See section
#      "Known Limitations with GCC 4"
#
%ifndef %{gcc_dir}
%   define  gcc_dir        /usr  # EDIT (see above)
%endif

# EDIT if needed: the preENV line adds library directories to the runtime
#      path.  You can adjust it, or add lines for other environment variables.
#      See: https://www.spec.org/cpu2017/Docs/config.html#preenv
#      and: https://gcc.gnu.org/onlinedocs/gcc/Environment-Variables.html
   preENV_LD_LIBRARY_PATH  = %{gcc_dir}/lib64/:%{gcc_dir}/lib/:/lib64
  #preENV_LD_LIBRARY_PATH  = %{gcc_dir}/lib64/:%{gcc_dir}/lib/:/lib64:%{ENV_LD_LIBRARY_PATH}
   SPECLANG                = %{gcc_dir}/bin/
   CC                      = $(SPECLANG)gcc     -std=c99   %{model}
   CXX                     = $(SPECLANG)g++     -std=c++03 %{model}
   FC                      = $(SPECLANG)gfortran           %{model}
   # How to say "Show me your version, please"
   CC_VERSION_OPTION       = -v
   CXX_VERSION_OPTION      = -v
   FC_VERSION_OPTION       = -v

default:
%if %{bits} == 64
   sw_base_ptrsize = 64-bit
   sw_peak_ptrsize = 64-bit
%else
   sw_base_ptrsize = 32-bit
   sw_peak_ptrsize = 32-bit
%endif

#--------- Portability --------------------------------------------------------
default:   # data model applies to all benchmarks
%if %{bits} == 32
    # Strongly recommended because at run-time, operations using modern file
    # systems may fail spectacularly and frequently (or, worse, quietly and
    # randomly) if a program does not accommodate 64-bit metadata.
    EXTRA_PORTABILITY = -D_FILE_OFFSET_BITS=64
%else
    EXTRA_PORTABILITY = -DSPEC_LP64
%endif

# Benchmark-specific portability (ordered by last 2 digits of bmark number)

500.perlbench_r,600.perlbench_s:  #lang='C'
%if %{bits} == 32
%   define suffix AARCH32
%else
%   define suffix AARCH64
%endif
   PORTABILITY    = -DSPEC_%{os}_%{suffix}

521.wrf_r,621.wrf_s:  #lang='F,C'
   CPORTABILITY  = -DSPEC_CASE_FLAG
   FPORTABILITY  = -fconvert=big-endian

523.xalancbmk_r,623.xalancbmk_s:  #lang='CXX'
   PORTABILITY   = -DSPEC_%{os}

526.blender_r:  #lang='CXX,C'
   PORTABILITY   = -funsigned-char -DSPEC_LINUX -std=gnu99

527.cam4_r,627.cam4_s:  #lang='F,C'
   PORTABILITY   = -DSPEC_CASE_FLAG

628.pop2_s:  #lang='F,C'
   PORTABILITY   = -DSPEC_CASE_FLAG -fconvert=big-endian

#-------- Tuning Flags common to Base and Peak --------------------------------

#
# Speed (OpenMP and Autopar allowed)
#
%if %{bits} == 32
   intspeed,fpspeed:
   #
   # Many of the speed benchmarks (6nn.benchmark_s) do not fit in 32 bits
   # If you wish to run SPECint2017_speed or SPECfp2017_speed, please use
   #
   #     runcpu --define bits=64
   #
   fail_build = 1
%else
   intspeed,fpspeed:
      EXTRA_OPTIMIZE = -fopenmp -DSPEC_OPENMP
   fpspeed:
      #
      # 627.cam4 needs a big stack; the preENV will apply it to all
      # benchmarks in the set, as required by the rules.
      #
      preENV_OMP_STACKSIZE = 1G
%endif

#--------  Baseline Tuning Flags ----------------------------------------------
#
default=base:         # flags for all base
   OPTIMIZE         = -O3

intrate,intspeed=base: # flags for integer base
    EXTRA_COPTIMIZE = -fno-strict-aliasing -fgnu89-inline
# Notes about the above
#  - 500.perlbench_r/600.perlbench_s needs -fno-strict-aliasing.
#  - 502.gcc_r/602.gcc_s             needs -fgnu89-inline or -z muldefs
#  - For 'base', all benchmarks in a set must use the same options.
#  - Therefore, all base benchmarks get the above.  See:
#       www.spec.org/cpu2017/Docs/runrules.html#BaseFlags
#       www.spec.org/cpu2017/Docs/benchmarks/500.perlbench_r.html
#       www.spec.org/cpu2017/Docs/benchmarks/502.gcc_r.html

#--------  Peak Tuning Flags ----------------------------------------------
default=peak:
   basepeak = yes  # if you develop some peak tuning, remove this line.
   #
   # -----------------------
   # About the -fno switches
   # -----------------------
   #
   # For 'base', this config file (conservatively) disables some optimizations.
   # You might want to try turning some of them back on, by creating a 'peak'
   # section here, with individualized benchmark options:
   #
   #        500.perlbench_r=peak:
   #           OPTIMIZE = this
   #        502.gcc_r=peak:
   #           OPTIMIZE = that
   #        503.bwaves_r=peak:
   #           OPTIMIZE = other   .....(and so forth)
   #
   # If you try it:
   #   - You must remove the 'basepeak' option, above.
   #   - You will need time and patience, to diagnose and avoid any errors.
   #   - perlbench is unlikely to work with strict aliasing
   #   - Some floating point benchmarks may get wrong answers, depending on:
   #         the particular chip
   #         the version of GCC
   #         other optimizations enabled
   #         -m32 vs. -m64
   #   - See: http://www.spec.org/cpu2017/Docs/config.html
   #   - and: http://www.spec.org/cpu2017/Docs/runrules.html

#------------------------------------------------------------------------------
# Tester and System Descriptions - EDIT all sections below this point
#------------------------------------------------------------------------------
#   For info about any field, see
#             https://www.spec.org/cpu2017/Docs/config.html#fieldname
#   Example:  https://www.spec.org/cpu2017/Docs/config.html#hw_memory
#-------------------------------------------------------------------------------

#--------- EDIT to match your version -----------------------------------------
default:
   sw_compiler001   = C/C++/Fortran: Version 6.2.0 of GCC, the
   sw_compiler002   = GNU Compiler Collection

#--------- EDIT info about you ------------------------------------------------
# To understand the difference between hw_vendor/sponsor/tester, see:
#     https://www.spec.org/cpu2017/Docs/config.html#test_sponsor
intrate,intspeed,fprate,fpspeed: # Important: keep this line
   hw_vendor          = My Corporation
   tester             = My Corporation
   test_sponsor       = My Corporation
   license_num        = nnn (Your SPEC license number)
#  prepared_by        = # Ima Pseudonym                       # Whatever you like: is never output

#--------- EDIT system availability dates -------------------------------------
intrate,intspeed,fprate,fpspeed: # Important: keep this line
                        # Example                             # Brief info about field
   hw_avail           = # Nov-2099                            # Date of LAST hardware component to ship
   sw_avail           = # Nov-2099                            # Date of LAST software component to ship

#--------- EDIT system information --------------------------------------------
intrate,intspeed,fprate,fpspeed: # Important: keep this line
                        # Example                             # Brief info about field
 # hw_cpu_name        = # Intel Xeon E9-9999 v9               # chip name
   hw_cpu_nominal_mhz = # 9999                                # Nominal chip frequency, in MHz
   hw_cpu_max_mhz     = # 9999                                # Max chip frequency, in MHz
 # hw_disk            = # 9 x 9 TB SATA III 9999 RPM          # Size, type, other perf-relevant info
   hw_model           = # TurboBlaster 3000                   # system model name
 # hw_nchips          = # 99                                  # number chips enabled
   hw_ncores          = # 9999                                # number cores enabled
   hw_ncpuorder       = # 1-9 chips                           # Ordering options
   hw_nthreadspercore = # 9                                   # number threads enabled per core
   hw_other           = # TurboNUMA Router 10 Gb              # Other perf-relevant hw, or "None"

#  hw_memory001       = # 999 GB (99 x 9 GB 2Rx4 PC4-2133P-R, # The 'PCn-etc' is from the JEDEC
#  hw_memory002       = # running at 1600 MHz)                # label on the DIMM.

   hw_pcache          = # 99 KB I + 99 KB D on chip per core  # Primary cache size, type, location
   hw_scache          = # 99 KB I+D on chip per 9 cores       # Second cache or "None"
   hw_tcache          = # 9 MB I+D on chip per chip           # Third  cache or "None"
   hw_ocache          = # 9 GB I+D off chip per system board  # Other cache or "None"

   fw_bios            = # American Megatrends 39030100 02/29/2016 # Firmware information
 # sw_file            = # ext99                               # File system
 # sw_os001           = # Linux Sailboat                      # Operating system
 # sw_os002           = # Distribution 7.2 SP1                # and version
   sw_other           = # TurboHeap Library V8.1              # Other perf-relevant sw, or "None"
 # sw_state           = # Run level 99                        # Software state.

# Note: Some commented-out fields above are automatically set to preliminary
# values by sysinfo
#       https://www.spec.org/cpu2017/Docs/config.html#sysinfo
# Uncomment lines for which you already know a better answer than sysinfo
```