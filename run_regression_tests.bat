@echo off

pushd ..\4coder-non-source\test_data
set run_path=%cd%\sample_files
set data_path=%cd%\input_data
popd

pushd ..\build
set build=%cd%
popd

pushd %run_path%
rem %build%\4ed -T %data_path%\test_bootstrap.4id
%build%\4ed -T %data_path%\gentest_capstress1.4id
popd

