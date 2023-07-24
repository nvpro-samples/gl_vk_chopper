cmake_minimum_required(VERSION 3.5)


get_filename_component(PROJNAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
Project(${PROJNAME})
Message(STATUS "-------------------------------")
Message(STATUS "Processing Project ${PROJNAME}:")

#####################################################################################
# look for nvpro_core 1) as a sub-folder 2) at some other locations
# this cannot be put anywhere else since we still didn't find setup.cmake yet
#
if(NOT BASE_DIRECTORY)

  find_path(BASE_DIRECTORY
    NAMES nvpro_core/cmake/setup.cmake
    PATHS ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/.. ${CMAKE_CURRENT_SOURCE_DIR}/../.. 
    REQUIRED
    DOC "Directory containing nvpro_core"
    )
endif()
if(EXISTS ${BASE_DIRECTORY}/nvpro_core/cmake/setup.cmake)
  include(${BASE_DIRECTORY}/nvpro_core/cmake/setup.cmake)
else()
  message(FATAL_ERROR "could not find base directory, please set BASE_DIRECTORY to folder containing nvpro_core")
endif()
    
_add_project_definitions(${PROJNAME})

#####################################################################################
# additions from packages needed for this sample
# add refs  in LIBRARIES_OPTIMIZED
# add refs  in LIBRARIES_DEBUG
# add files in PACKAGE_SOURCE_FILES
#

_add_package_OpenGL()
_add_package_VulkanSDK()
_add_package_ShaderC()

#####################################################################################
# process the rest of some cmake code that needs to be done *after* the packages add
_add_nvpro_core_lib()

#####################################################################################
# Source files for this project
#
file(GLOB SOURCE_FILES *.cpp *.hpp *.inl *.h *.c)
file(GLOB GLSL_FILES shaders/*.glsl)

file(GLOB IMAGE_FILES images/*.*)


#####################################################################################
# Executable
#
if(WIN32 AND NOT GLUT_FOUND)
  add_definitions(/wd4244) #remove double to float conversion warning
  add_definitions(/wd4305) #remove double to float truncation warning
endif()

add_executable(${PROJNAME} ${SOURCE_FILES} ${COMMON_SOURCE_FILES} ${PACKAGE_SOURCE_FILES} ${GLSL_FILES})

if(UNIX)
  add_definitions(-g -std=c++11)
  set(UNIXLINKLIBS dl png assimp pthread)
else()
  set(UNIXLINKLIBS)
endif()


#####################################################################################
# common source code needed for this sample
#
source_group(common FILES 
  ${COMMON_SOURCE_FILES}
  ${PACKAGE_SOURCE_FILES}
)
source_group(shaders FILES 
  ${GLSL_FILES}
)

#####################################################################################
# Linkage
#
target_link_libraries(${PROJNAME} nvpro_core ${PLATFORM_LIBRARIES} ${UNIXLINKLIBS})

foreach(DEBUGLIB ${LIBRARIES_DEBUG})
  target_link_libraries(${PROJECT_NAME} debug ${DEBUGLIB})
endforeach(DEBUGLIB)

foreach(RELEASELIB ${LIBRARIES_OPTIMIZED})
  target_link_libraries(${PROJECT_NAME} optimized ${RELEASELIB})
endforeach(RELEASELIB)

#####################################################################################
# copies binaries that need to be put next to the exe files (ZLib, etc.)
#

_finalize_target( ${PROJNAME} )

# additional copies for standalone run from install folder
install(FILES ${GLSL_FILES} CONFIGURATIONS Release DESTINATION "bin_${ARCH}/GLSL_${PROJNAME}")
install(FILES ${GLSL_FILES} CONFIGURATIONS Debug DESTINATION "bin_${ARCH}_debug/GLSL_${PROJNAME}")
install(FILES "chopper_pack32.vks" CONFIGURATIONS Release DESTINATION "bin_${ARCH}/resources_${PROJNAME}")
install(FILES "chopper_pack32.vks" CONFIGURATIONS Debug DESTINATION "bin_${ARCH}_debug/resources_${PROJNAME}")
install(FILES ${IMAGE_FILES} CONFIGURATIONS Release DESTINATION "bin_${ARCH}/resources_${PROJNAME}")
install(FILES ${IMAGE_FILES} CONFIGURATIONS Debug DESTINATION "bin_${ARCH}_debug/resources_${PROJNAME}")