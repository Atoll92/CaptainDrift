cmake_minimum_required(VERSION 3.22)
project(CaptainDrift VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build universal binary (arm64 + x86_64) for Rosetta compatibility
set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64")

# --- JUCE ---
# Use local JUCE copy for macOS, FetchContent for CI/Windows
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/JUCE/CMakeLists.txt")
    add_subdirectory(JUCE)
else()
    include(FetchContent)
    FetchContent_Declare(
        juce
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG        8.0.4
        GIT_SHALLOW    TRUE
    )
    FetchContent_MakeAvailable(juce)
endif()

# --- Plugin Target ---
juce_add_plugin(CaptainDrift
    COMPANY_NAME          "CaptainDriftAudio"
    BUNDLE_ID             "com.captaindriftaudio.captaindrift"
    PLUGIN_MANUFACTURER_CODE Vlnt
    PLUGIN_CODE           Cdft
    FORMATS               VST3 AU Standalone
    PRODUCT_NAME          "CaptainDrift"
    PLUGIN_NAME           "CaptainDrift"
    DESCRIPTION           "Generative MIDI engine for infinite ambient pads"
    IS_SYNTH              TRUE
    NEEDS_MIDI_INPUT      TRUE
    NEEDS_MIDI_OUTPUT     TRUE
    IS_MIDI_EFFECT        FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE
)

target_sources(CaptainDrift PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/Engine/ParameterLayout.cpp
    Source/Engine/ScaleQuantizer.cpp
    Source/Engine/PhaseAccumulator.cpp
    Source/Engine/EvolutionCurve.cpp
    Source/Engine/MicrotonalPitchBend.cpp
    Source/Engine/DriftVoice.cpp
    Source/Engine/GenerativeEngine.cpp
    Source/GUI/DriftLookAndFeel.cpp
    Source/GUI/DriftBackground.cpp
)

target_include_directories(CaptainDrift PRIVATE Source)

target_compile_definitions(CaptainDrift PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
)

target_link_libraries(CaptainDrift
    PRIVATE
        juce::juce_audio_utils
        juce::juce_gui_extra
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)
