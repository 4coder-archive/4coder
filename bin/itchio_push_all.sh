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
dir=../current_dist_all_os

butler push $flags $dir/4coder-alpha-$fv-win-x64.zip   4coder/4coder:win-x64-alpha
butler push $flags $dir/4coder-alpha-$fv-linux-x64.zip 4coder/4coder:linux-x64-alpha
butler push $flags $dir/4coder-alpha-$fv-mac-x64.zip   4coder/4coder:osx-x64-alpha
butler push $flags $dir/4coder-alpha-$fv-win-x86.zip   4coder/4coder:win-x86-alpha
butler push $flags $dir/4coder-alpha-$fv-linux-x86.zip 4coder/4coder:linux-x86-alpha
butler push $flags $dir/4coder-alpha-$fv-mac-x86.zip   4coder/4coder:osx-x86-alpha

butler push $flags $dir/4coder-alpha-$fv-super-win-x64.zip   4coder/4coder:win-x64-super
butler push $flags $dir/4coder-alpha-$fv-super-linux-x64.zip 4coder/4coder:linux-x64-super
butler push $flags $dir/4coder-alpha-$fv-super-mac-x64.zip   4coder/4coder:osx-x64-super
butler push $flags $dir/4coder-alpha-$fv-super-win-x86.zip   4coder/4coder:win-x86-super
butler push $flags $dir/4coder-alpha-$fv-super-linux-x86.zip 4coder/4coder:linux-x86-super
butler push $flags $dir/4coder-alpha-$fv-super-mac-x86.zip   4coder/4coder:osx-x86-super

fi
