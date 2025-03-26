cmake_minimum_required(VERSION 3.25)

# These variables should be set before the first project() command is called.
# Additionally, they should be set as cache variables so that their values can be overridden.
# References:
# - https://cmake.org/cmake/help/v3.25/variable/CMAKE_OSX_ARCHITECTURES.html
# - https://cmake.org/cmake/help/v3.25/variable/CMAKE_OSX_DEPLOYMENT_TARGET.html
set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "Build architectures for macOS")
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.12" CACHE STRING "Minimum macOS version required")

project(simple_reverb VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

add_subdirectory(lib/JUCE)

juce_add_plugin(simple_reverb
        COMPANY_NAME "Vassilis Kontses"
        COMPANY_WEBSITE "https://github.com/Kontses"
        COMPANY_EMAIL "libys97@gmail.com"
        BUNDLE_ID "dev.kontses.${PROJECT_NAME}"
        PLUGIN_MANUFACTURER_CODE "Knts"
        PLUGIN_CODE "Srvb"
        FORMATS Standalone VST3 AU
        COPY_PLUGIN_AFTER_BUILD TRUE
        PLUGIN_NAME "3D Reverb"
)

target_compile_features(simple_reverb PUBLIC cxx_std_20)

target_compile_definitions(simple_reverb PUBLIC JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0)

target_sources(
    simple_reverb
    PRIVATE
        src/PluginEditor.cpp
        src/PluginProcessor.cpp
        src/ui/EditorContent.cpp
        src/ui/Dial.cpp
        src/ui/FreezeButton.cpp
        src/ui/EditorLnf.cpp
        src/ui/NumericInputFilter.h
)

juce_add_binary_data(binary_data SOURCES
        res/FreezeIcon.svg
        res/UbuntuRegular.ttf
)

target_link_libraries(
    simple_reverb
    PRIVATE
        binary_data
        juce::juce_audio_utils
        juce::juce_dsp
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)
