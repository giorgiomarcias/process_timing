cmake_minimum_required(VERSION 3.3)

project(process_timing)

option(ATTACH_SOURCES "When generating an IDE project, add process_timing header files to project sources." ON)



set(hdr_dir ${PROJECT_SOURCE_DIR}/include)

set(hdr_main_files
	${hdr_dir}/process_timing/process_timing.hpp
)
source_group("process_timing" FILES ${hdr_main_files})



add_library(${PROJECT_NAME} INTERFACE)

set(required_cxx_features
	cxx_alias_templates
)

target_compile_features(${PROJECT_NAME} INTERFACE ${required_cxx_features})
target_include_directories(${PROJECT_NAME} INTERFACE ${hdr_dir})
if(ATTACH_SOURCES)
	target_sources(${PROJECT_NAME} INTERFACE ${hdr_main_files})
endif()
