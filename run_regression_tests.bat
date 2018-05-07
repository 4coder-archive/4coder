@echo off
pushd ..\4coder-non-source\test_data
set run_path=%cd%\sample_files
set data_path=%cd%\input_data
popd
pushd ..\build
set build=%cd%
popd
pushd %run_path%
%build%\4ed -T %data_path%\test_load_rome_txt.4id
popd
