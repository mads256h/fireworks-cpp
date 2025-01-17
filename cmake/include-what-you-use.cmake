function(try_enable_include_what_you_use target mapping_file)
    find_program(IWYU NAMES include-what-you-use iwyu)

    if (NOT IWYU MATCHES "IWYU-NOTFOUND")
        get_filename_component(absolute_mapping_file ${mapping_file} ABSOLUTE)
        set(iwyu_with_options
            ${IWYU}
            -Xiwyu --regex llvm
            -Xiwyu --mapping_file=${absolute_mapping_file})
        set_property(TARGET ${target} PROPERTY CXX_INCLUDE_WHAT_YOU_USE ${iwyu_with_options})
    else()
        message("include-what-you-use not found. Continuing...")
    endif()
endfunction()