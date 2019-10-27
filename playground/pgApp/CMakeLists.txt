cmake_minimum_required (VERSION 3.15.4)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

project(pgApp CXX)

add_compile_definitions(
	_ENABLE_ATOMIC_ALIGNMENT_FIX=1
	RIGHT_HANDED=0
	RIGHT_HANDED_FRONT_CCW=1
	LEFT_HANDED_FRONT_CCW=0
	#_CRTDBG_MAP_ALLOC=1
	)

set (DE_TOP_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../..")

set(SHADERS
    assets/shaders/env_map.vsh
    assets/shaders/env_map.psh
    assets/cube.vsh
    assets/cube.psh
    assets/cubetex.vsh
    assets/cubetex.psh
    ../../DiligentFX/Shaders/PostProcess/ToneMapping/public/ToneMappingStructures.fxh
    ../../DiligentFX/Shaders/PostProcess/ToneMapping/public/ToneMapping.fxh
    ../../DiligentFX/Shaders/Common/public/BasicStructures.fxh
)

file(GLOB_RECURSE ASSETS_ALL
	RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/assets/*.*)

set(ASSETS ${ASSETS_ALL})
list(FILTER ASSETS EXCLUDE REGEX ".*\.hlsl$")
list(FILTER ASSETS EXCLUDE REGEX ".*\.obj$")

file(GLOB_RECURSE SRC_SOURCE
	RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/src/*.*)
	
set(ZUPPLY_SOURCE	
	${DE_TOP_PATH}/thirdparty/zupply/src/zupply.hpp
	${DE_TOP_PATH}/thirdparty/zupply/src/zupply.cpp)
	
source_group(TREE ${DE_TOP_PATH} FILES ${ZUPPLY_SOURCE})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SRC_SOURCE})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${ASSETS_ALL})

set(SOURCE ${SOURCE} ${ZUPPLY_SOURCE})
set(SOURCE ${SOURCE} ${SRC_SOURCE})

set(PROJECT_NAME_TMP "pgApp")

add_sample_app(${PROJECT_NAME_TMP} "playground" "${SOURCE}" "${INCLUDE}" "${SHADERS}" "${ASSETS}")

#set_target_properties(${PROJECT_NAME_TMP} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
#set_target_properties(${PROJECT_NAME_TMP} PROPERTIES VS_DEBUGGER_COMMAND_ARGUMENTS "-c ./conf/crytek-sponza.3dgep")
set_target_properties(${PROJECT_NAME_TMP} PROPERTIES VS_DEBUGGER_COMMAND_ARGUMENTS "-mode d3d11 -width 1280 -height 720")

target_link_libraries(${PROJECT_NAME_TMP} PRIVATE Diligent-TextureLoader 
			Diligent-AssetLoader DiligentFX
			assimp
			)


target_include_directories(${PROJECT_NAME_TMP} PRIVATE
    ../../DiligentFX/Shaders/PostProcess/ToneMapping/public/
)

######
target_link_directories(${PROJECT_NAME_TMP} PRIVATE ""
			)

target_include_directories(${PROJECT_NAME_TMP} PRIVATE 
						${CMAKE_CURRENT_SOURCE_DIR}/src
						${DE_TOP_PATH}/thirdparty/
						${DE_TOP_PATH}/thirdparty/glm-0.9.9.6/
						#${DE_TOP_PATH}/thirdparty/assimp-5.0.0/include
						#${CMAKE_BINARY_DIR}/thirdparty/assimp-5.0.0/include # assimp generated config.h			
						${DE_TOP_PATH}/thirdparty/entt/src
						${DE_TOP_PATH}/thirdparty/fibertaskinglib/include
						${DE_TOP_PATH}/thirdparty/taskflow
						${DE_TOP_PATH}/thirdparty/flythrough_camera
						${DE_TOP_PATH}/thirdparty/arcball_camera
						${DE_TOP_PATH}/thirdparty/argparse
						${DE_TOP_PATH}/thirdparty/zupply
						)

######
add_custom_command(TARGET ${PROJECT_NAME_TMP} PRE_BUILD 
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/../../DiligentFX/Shaders/PostProcess/ToneMapping/public/ToneMappingStructures.fxh" "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/../../DiligentFX/Shaders/PostProcess/ToneMapping/public/ToneMapping.fxh" "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/../../DiligentFX/Shaders/Common/public/BasicStructures.fxh" "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders"
)

