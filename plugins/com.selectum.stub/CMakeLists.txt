qt_add_plugin(com_selectum_stub
    CLASS_NAME StubPlugin
    stub_plugin.cpp
    stub_plugin.h
)

set_target_properties(com_selectum_stub PROPERTIES
    OUTPUT_NAME "stub"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins/com.selectum.stub"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins/com.selectum.stub"
)

target_link_libraries(com_selectum_stub
    PRIVATE
        selectum_abi
        Qt::Core
)

# Deploy plugin.json beside the built module so PluginHost/tests can scan it.
add_custom_command(TARGET com_selectum_stub POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/plugin.json"
        "${CMAKE_BINARY_DIR}/plugins/com.selectum.stub/plugin.json"
    COMMENT "Deploy stub plugin.json"
)
