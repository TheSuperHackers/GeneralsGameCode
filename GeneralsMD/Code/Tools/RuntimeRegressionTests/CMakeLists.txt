add_executable(z_runtime_regression_tests)

target_sources(z_runtime_regression_tests PRIVATE
    RuntimeRegressionTests.cpp
)

target_link_libraries(z_runtime_regression_tests PRIVATE
	binkstub
	comctl32
	core_debug
	core_profile_legacy
	d3d8
	d3dx8
	dinput8
	dxguid
	imm32
	milesstub
	vfw32
	winmm
	z_gameengine
	z_gameenginedevice
	zi_always
)
