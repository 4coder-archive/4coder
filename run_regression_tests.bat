@echo off
pushd ..\4coder-non-source\test_data
set run_path=%cd%\sample_files
set data_path=%cd%\input_data
popd
pushd ..\build
set build=%cd%
popd
pushd %run_path%
%build%\4ed -T %data_path%\test_load_FONT_COURIER_NEW_28_c.4id
%build%\4ed -T %data_path%\gentest_capstress1.4id
%build%\4ed -T %data_path%\gentest_capstress2.4id
%build%\4ed -T %data_path%\gentest_capstress3.4id
%build%\4ed -T %data_path%\gentest_capstress4.4id
%build%\4ed -T %data_path%\gentest_capstress5.4id
%build%\4ed -T %data_path%\gentest_dupline0_0.4id
%build%\4ed -T %data_path%\gentest_dupline0_1.4id
%build%\4ed -T %data_path%\gentest_dupline1_0.4id
%build%\4ed -T %data_path%\gentest_dupline1_1.4id
%build%\4ed -T %data_path%\gentest_dupline_readonly.4id
popd
