cmake_minimum_required (VERSION 3.6)

project(pgApp CXX)

set(SHADERS
    assets/shaders/env_map.vsh
    assets/shaders/env_map.psh
    ../../DiligentFX/Shaders/PostProcess/ToneMapping/public/ToneMappingStructures.fxh
    ../../DiligentFX/Shaders/PostProcess/ToneMapping/public/ToneMapping.fxh
    ../../DiligentFX/Shaders/Common/public/BasicStructures.fxh
)

file(GLOB BOOM_BOX_MODEL LIST_DIRECTORIES false assets/models/BoomBoxWithAxes/*.*)
file(GLOB CESIUM_MAN_MODEL LIST_DIRECTORIES false assets/models/CesiumMan/*.*)
file(GLOB DAMAGED_HELMENT_MODEL LIST_DIRECTORIES false assets/models/DamagedHelmet/*.*)
file(GLOB FLIGHT_HELMENT_MODEL LIST_DIRECTORIES false assets/models/FlightHelmet/*.*)
file(GLOB METAL_ROUGH_SPHERES_MODEL LIST_DIRECTORIES false assets/models/MetalRoughSpheres/*.*)
file(GLOB NORMAL_TANGENT_TEST_MODEL LIST_DIRECTORIES false assets/models/NormalTangentTest/*.*)
file(GLOB Sponza_BOX_MODEL LIST_DIRECTORIES false assets/models/Sponza/*.*)
file(GLOB VirtualCity_BOX_MODEL LIST_DIRECTORIES false assets/models/VirtualCity/*.*)

set(MODELS ${BOOM_BOX_MODEL} ${CESIUM_MAN_MODEL} ${DAMAGED_HELMENT_MODEL}
           ${FLIGHT_HELMENT_MODEL} ${METAL_ROUGH_SPHERES_MODEL} ${NORMAL_TANGENT_TEST_MODEL} ${Sponza_BOX_MODEL} ${VirtualCity_BOX_MODEL})
set(TEXTURES assets/textures/papermill.ktx)
set(ASSETS
    ${MODELS}
    ${TEXTURES}
)

file(GLOB_RECURSE SRC_SOURCE
	RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/src/*.*)
	
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SRC_SOURCE})

set(SOURCE ${SOURCE} ${SRC_SOURCE})

set(PROJECT_NAME_TMP "pgApp")

add_sample_app(${PROJECT_NAME_TMP} "playground" "${SOURCE}" "${INCLUDE}" "${SHADERS}" "${ASSETS}")

#set_target_properties(${PROJECT_NAME_TMP} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
#set_target_properties(${PROJECT_NAME_TMP} PROPERTIES VS_DEBUGGER_COMMAND_ARGUMENTS "-c ./conf/crytek-sponza.3dgep")
set_target_properties(${PROJECT_NAME_TMP} PROPERTIES VS_DEBUGGER_COMMAND_ARGUMENTS "-mode d3d11 -width 1280 -height 720")

target_link_libraries(${PROJECT_NAME_TMP} PRIVATE Diligent-TextureLoader 
			Diligent-AssetLoader DiligentFX
			#assimp
			)


target_include_directories(${PROJECT_NAME_TMP} PRIVATE
    ../../DiligentFX/Shaders/PostProcess/ToneMapping/public/
)

######
set (DE_TOP_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../..")

target_link_directories(${PROJECT_NAME_TMP} PRIVATE ""
			)

target_include_directories(${PROJECT_NAME_TMP} PRIVATE 
						${DE_TOP_PATH}/thirdparty/
						${DE_TOP_PATH}/thirdparty/glm-0.9.9.6/
						#${DE_TOP_PATH}/thirdparty/assimp-5.0.0/include
						${CMAKE_BINARY_DIR}/thirdparty/assimp-5.0.0/include # assimp generated config.h			
						${DE_TOP_PATH}/thirdparty/entt/src
						${DE_TOP_PATH}/thirdparty/fibertaskinglib/include
						${DE_TOP_PATH}/thirdparty/taskflow
						${DE_TOP_PATH}/thirdparty/flythrough_camera
						${DE_TOP_PATH}/thirdparty/arcball_camera
						${DE_TOP_PATH}/thirdparty/argparse
						)

######
add_custom_command(TARGET ${PROJECT_NAME_TMP} PRE_BUILD 
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/../../DiligentFX/Shaders/PostProcess/ToneMapping/public/ToneMappingStructures.fxh" "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/../../DiligentFX/Shaders/PostProcess/ToneMapping/public/ToneMapping.fxh" "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/../../DiligentFX/Shaders/Common/public/BasicStructures.fxh" "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders"
)


set_source_files_properties(${SHADERS} PROPERTIES
    VS_DEPLOYMENT_LOCATION "shaders"
    MACOSX_PACKAGE_LOCATION Resources/shaders
)

set_source_files_properties(${TEXTURES} PROPERTIES
    VS_DEPLOYMENT_LOCATION "textures"
    MACOSX_PACKAGE_LOCATION "Resources/textures"
)

set_source_files_properties(${BOOM_BOX_MODEL} PROPERTIES
    VS_DEPLOYMENT_LOCATION "models/BoomBoxWithAxes"
    MACOSX_PACKAGE_LOCATION "Resources/models/BoomBoxWithAxes"
)

set_source_files_properties(${CESIUM_MAN_MODEL} PROPERTIES
    VS_DEPLOYMENT_LOCATION "models/CesiumMan"
    MACOSX_PACKAGE_LOCATION "Resources/models/CesiumMan"
)

set_source_files_properties(${DAMAGED_HELMENT_MODEL} PROPERTIES
    VS_DEPLOYMENT_LOCATION "models/DamagedHelmet"
    MACOSX_PACKAGE_LOCATION "Resources/models/DamagedHelmet"
)

set_source_files_properties(${FLIGHT_HELMENT_MODEL} PROPERTIES
    VS_DEPLOYMENT_LOCATION "models/FlightHelmet"
    MACOSX_PACKAGE_LOCATION "Resources/models/FlightHelmet"
)

set_source_files_properties(${METAL_ROUGH_SPHERES_MODEL} PROPERTIES
    VS_DEPLOYMENT_LOCATION "models/MetalRoughSpheres"
    MACOSX_PACKAGE_LOCATION "Resources/models/MetalRoughSpheres"
)

set_source_files_properties(${NORMAL_TANGENT_TEST_MODEL} PROPERTIES
    VS_DEPLOYMENT_LOCATION "models/NormalTangentTest"
    MACOSX_PACKAGE_LOCATION "Resources/models/NormalTangentTest"
)

set_source_files_properties(${Sponza_BOX_MODEL} PROPERTIES
    VS_DEPLOYMENT_LOCATION "models/Sponza"
    MACOSX_PACKAGE_LOCATION "Resources/models/Sponza"
)

set_source_files_properties(${VirtualCity_BOX_MODEL} PROPERTIES
    VS_DEPLOYMENT_LOCATION "models/VirtualCity"
    MACOSX_PACKAGE_LOCATION "Resources/models/VirtualCity"
)


source_group("assets" FILES ${ASSETS})
source_group("assets\\shaders" FILES ${SHADERS})
source_group("assets\\textures" FILES ${TEXTURES})
source_group("assets\\models\\BoomBoxWithAxes" FILES ${BOOM_BOX_MODEL})
source_group("assets\\models\\CesiumMan" FILES ${CESIUM_MAN_MODEL})
source_group("assets\\models\\DamagedHelmet" FILES ${DAMAGED_HELMENT_MODEL})
source_group("assets\\models\\FlightHelmet" FILES ${FLIGHT_HELMENT_MODEL})
source_group("assets\\models\\MetalRoughSpheres" FILES ${METAL_ROUGH_SPHERES_MODEL})
source_group("assets\\models\\NormalTangentTest" FILES ${NORMAL_TANGENT_TEST_MODEL})
source_group("assets\\models\\Sponza" FILES ${Sponza_BOX_MODEL})
source_group("assets\\models\\VirtualCity" FILES ${VirtualCity_BOX_MODEL})

