#! /usr/bin/env bash
#
# This script runs each systems test and each unit test, suppressing all
# output but reporting whether the test passed or failed.  The system tests
# that get run are hardcoded in the script; you need to add a new section
# if a new test is created.
#
# The script assumes that it is in a subdirectory of the eclipse workspace folder
# (I have it in my sandbox directory)

if test "${0%/*}" != "$0"
then
    cd "${0%/*}"
fi
cd ../../ # Assumes the script is in a subdirectory of the eclipse workspace folder
wd=$PWD

echo $wd

function runandecho() {
    testname=$1
    shift
    if $* 1> /dev/null 2>/dev/null
    then
        echo "$testname passed"
    else
        echo "$testname FAILED"
    fi
}

# Check if compiled with MPI
#mpistring="$(petesandbox/Debug/pv 2>/dev/null | egrep PV_USE_MPI)"
#if test "$mpistring" = "PV_USE_MPI is set"
#then
   echo "assumes PetaVision compiled using PV_USE_MPI"
   usempi=1
   function mpirunandecho() {
       testname=$1
       shift
       if mpirun -np 4 $* 1> /dev/null 2>/dev/null
       then
           echo "$testname with four processes passed"
       else
           echo "$testname with four processes FAILED"
       fi
   }
#elif test "$mpistring" = "PV_USE_MPI is not set"
#then
#    echo "PetaVision compiled with PV_USE_MPI turned off"
#    usempi=0
#    function mpirunandecho() {
#        false
#    }
#else
#    exit 1
#fi

testname=ArborSystemTest
arglist="-p input/test_arbors.params"
cd "$testname"
runandecho $testname Debug/$testname $arglist
mpirunandecho $testname Debug/$testname $arglist
cd $wd

testname=CheckpointSystemTest
arglist=""
cd "$testname"
runandecho $testname Debug/$testname $arglist
mpirunandecho $testname Debug/$testname $arglist
cd $wd

testname=DatastoreDelayTest
arglist="-p input/DatastoreDelayTest.params"
cd "$testname"
runandecho $testname Debug/$testname $arglist
mpirunandecho $testname Debug/$testname $arglist
cd $wd

testname=FourByFourGenerativeTest
arglist=""
cd "$testname"
runandecho $testname Debug/$testname $arglist
cd $wd

testname=FourByFourTopDownTest
arglist=""
cd "$testname"
runandecho $testname Debug/$testname $arglist
cd $wd

testname=GenerativeConnTest
arglist=""
cd "$testname"
runandecho $testname Debug/$testname $arglist
mpirunandecho $testname Debug/$testname $arglist
cd $wd

testname=InitWeightsTest
arglist="-p input/test_initweights.params"
cd "$testname"
runandecho $testname Debug/$testname $arglist
mpirunandecho $testname Debug/$testname $arglist
cd $wd

testname=KernelTest
arglist="-p input/test_kernel.params"
cd "$testname"
runandecho $testname Debug/$testname $arglist
mpirunandecho $testname Debug/$testname $arglist
cd $wd

testname=LayerRestartTest
arglist=""
cd "$testname"
runandecho $testname Debug/$testname $arglist
mpirunandecho $testname Debug/$testname $arglist
cd $wd

testname=MPITest2
arglist="-p input/MPI_test.params -n 100"
cd "$testname"
runandecho $testname Debug/$testname $arglist
mpirunandecho $testname Debug/$testname $arglist
cd $wd

testname=PlasticConnTest
cd "$testname"
arglist="-p input/PlasticConnTest.params"
runandecho $testname Debug/$testname $arglist
mpirunandecho $testname Debug/$testname $arglist
cd "$wd"

testname=ReadArborFileTest
cd "$testname"
arglist="-p input/ReadArborFileTest.params"
runandecho $testname Debug/$testname $arglist
mpirunandecho $testname Debug/$testname $arglist
cd "$wd"

echo $PWD
cd "./PetaVision/tests"
make runtests 2>/dev/null | egrep 'passed|failed'
if test $usempi -eq 1
then
    make runMPItests 2>/dev/null | egrep 'passed|failed'
fi
cd $wd
