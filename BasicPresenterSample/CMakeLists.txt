

//hahaahahahahaha
#//////////////////////////////////////////////////////////////////////////////
#//
#//  Copyright (C) 2015 zSpace, Inc.  All Rights Reserved.
#//
#//  File:       CMakeLists.txt
#//  Content:    CMake file to build zView presenter sample.
#//
#//////////////////////////////////////////////////////////////////////////////

# Input Files
set (
    SOURCE_FILES

    BasicPresenterSample.cpp
    glew.cpp
)

# Source Groups
source_group ("" FILES ${SOURCE_FILES})

# Build Target
set (PROJECT_NAME "BasicPresenterSample")
add_executable (${PROJECT_NAME} ${SOURCE_FILES})
set_target_properties (${PROJECT_NAME} PROPERTIES WIN32_EXECUTABLE true)
set_property (TARGET ${PROJECT_NAME} PROPERTY FOLDER zView/Samples)

add_definitions(-DGLEW_STATIC)

# Include Directories
target_include_directories (
    ${PROJECT_NAME}

    PRIVATE
    ./
)

# Target Link Libraries
target_link_libraries (
    ${PROJECT_NAME}

    zViewApi

    zSpaceCoreApi

    glm

    openGL32
    glu32
)

set (
    SHADERS

    "BasicPresenterSampleVertexShader.glsl"
    "BasicPresenterSampleFragmentShader.glsl"

    "BasicPresenterSampleAugmentedRealityModeMaskVertexShader.glsl"
    "BasicPresenterSampleAugmentedRealityModeMaskFragmentShader.glsl"

    "BasicPresenterSampleAugmentedRealityModeBackgroundVertexShader.glsl"
    "BasicPresenterSampleAugmentedRealityModeBackgroundFragmentShader.glsl"
)

foreach (SHADER ${SHADERS})
    configure_file (
        ${SHADER} ${OUTPUT_BIN_DIRECTORY}/Debug/${SHADER} COPYONLY
    )
    configure_file (
        ${SHADER} ${OUTPUT_BIN_DIRECTORY}/Release/${SHADER} COPYONLY
    )
endforeach()

