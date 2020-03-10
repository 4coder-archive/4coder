#!/bin/sh

if [ "$#" -lt "3" ]
then
echo need 3 parameters
exit
else

fake=$1
maj=$2
min=$3

vr=$fake.$maj.$min
fv=$fake-$maj-$min

flags="--fix-permissions --userversion=$vr"
dir=../distributions

butler push $flags $dir/demo_x86/4coder-$fv-demo-linux-x86.zip   4coder/4coder:linux-x86-demo
butler push $flags $dir/super_x86/4coder-$fv-super-linux-x86.zip 4coder/4coder:linux-x86

fi
