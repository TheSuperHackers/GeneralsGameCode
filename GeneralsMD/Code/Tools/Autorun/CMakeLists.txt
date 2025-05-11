macro(setup_autorun appname)
    add_executable(${appname} WIN32)

    target_link_libraries(${appname} PRIVATE
        corei_autorun
        zi_always
    )
endmacro()

setup_autorun(z_autorun_en)
setup_autorun(z_autorun_de)
setup_autorun(z_autorun_fr)
setup_autorun(z_autorun_es)
set_target_properties(z_autorun_en PROPERTIES OUTPUT_NAME autorun_en)
set_target_properties(z_autorun_de PROPERTIES OUTPUT_NAME autorun_de)
set_target_properties(z_autorun_fr PROPERTIES OUTPUT_NAME autorun_fr)
set_target_properties(z_autorun_es PROPERTIES OUTPUT_NAME autorun_es)

# TODO: Configure different resources for languages. See Autorun.dsp
