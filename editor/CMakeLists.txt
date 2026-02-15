add_executable(FreshEditor main.cpp)
target_link_libraries(FreshEditor FreshEngine)

if(MSVC)
    target_compile_options(FreshEditor PRIVATE /W4 /WX)
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        target_compile_options(FreshEditor PRIVATE /O2 /Oi /Ot /GL /Gy /GS- /fp:fast /arch:AVX2)
        target_link_options(FreshEditor PRIVATE /LTCG /OPT:REF /OPT:ICF /SUBSYSTEM:WINDOWS)
        set_target_properties(FreshEditor PROPERTIES WIN32_EXECUTABLE TRUE)
    endif()
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_compile_options(FreshEditor PRIVATE /ZI /Od /RTC1)
    endif()
endif()

install(TARGETS FreshEditor DESTINATION bin)

# Copy shaders to build directory
file(GLOB SHADER_FILES "${CMAKE_SOURCE_DIR}/shaders/*.vert" 
                       "${CMAKE_SOURCE_DIR}/shaders/*.frag")
foreach(SHADER ${SHADER_FILES})
    get_filename_component(SHADER_NAME ${SHADER} NAME)
    add_custom_command(
        TARGET FreshEditor POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:FreshEditor>/shaders"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${SHADER}" "$<TARGET_FILE_DIR:FreshEditor>/shaders/${SHADER_NAME}"
        COMMENT "Copying shader ${SHADER_NAME} for FreshEditor"
        VERBATIM
    )
endforeach()
