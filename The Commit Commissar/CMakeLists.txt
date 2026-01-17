cmake_minimum_required(VERSION 3.31)
include(FetchContent)
project(TheCommitCommissar)

set(CMAKE_CXX_STANDARD 23)

file(GLOB SRC
        "*.cpp"
)

set(FETCHCONTENT_BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/dependencies")
FetchContent_Declare(WolfEngineContent
        GIT_REPOSITORY https://github.com/arthur-monteiro/WolfEngine-2.0
        GIT_TAG c228e44ca6703368dcf545fab25f35ffd762e052
        SOURCE_SUBDIR "__skip_subdirectory__")
FetchContent_MakeAvailable(WolfEngineContent)

# Includes Wolf libs
include_directories("dependencies/wolfenginecontent-src/Common")
include_directories("dependencies/wolfenginecontent-src/GraphicAPIBroker/Public")
include_directories("dependencies/wolfenginecontent-src/Wolf-Engine-2.0")

# Includes third parties
include_directories("dependencies/wolfenginecontent-src/ThirdParty/xxh64")
include_directories("dependencies/wolfenginecontent-src/ThirdParty/glm")
include_directories("dependencies/wolfenginecontent-src/ThirdParty/GLFW/include")
include_directories("dependencies/wolfenginecontent-src/ThirdParty/tiny_obj")
include_directories("dependencies/wolfenginecontent-src/ThirdParty/FrustumCull")
include_directories("dependencies/wolfenginecontent-src/ThirdParty/stb_image")
include_directories("dependencies/wolfenginecontent-src/ThirdParty/UltraLight/include")
include_directories("dependencies/wolfenginecontent-src/ThirdParty/vulkan/Include")
include_directories("dependencies/wolfenginecontent-src/ThirdParty/Tracy/tracy")

link_directories("dependencies/wolfenginecontent-src/x64/Debug/lib")
link_directories("dependencies/wolfenginecontent-src/ThirdParty/vulkan/Lib")
link_directories("dependencies/wolfenginecontent-src/ThirdParty/GLFW/lib-vc2019")
link_directories("dependencies/wolfenginecontent-src/ThirdParty/UltraLight/lib")
link_directories("../x64/Debug")

add_subdirectory(dependencies/traypp EXCLUDE_FROM_ALL)
link_libraries(tray)
add_subdirectory("dependencies/wolfenginecontent-src")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(TheCommitCommissar ${SRC})

target_compile_definitions(TheCommitCommissar PUBLIC WOLF_VULKAN)
target_compile_definitions(TheCommitCommissar PUBLIC MATERIAL_DEBUG)
target_compile_definitions(TheCommitCommissar PUBLIC TRACY_ENABLE)
target_compile_definitions(TheCommitCommissar PUBLIC TRACY_ON_DEMAND)

if (RESOURCE_TRACKING OR RESOURCE_DEBUG)
    target_compile_definitions(
            TheCommitCommissar
            PRIVATE
            $<$<BOOL:RESOURCE_TRACKING>:RESOURCE_TRACKING>
            $<$<BOOL:RESOURCE_DEBUG>:RESOURCE_DEBUG>
    )
endif()

target_link_libraries(TheCommitCommissar vulkan-1.lib)
target_link_libraries(TheCommitCommissar glfw3.lib)
target_link_libraries(TheCommitCommissar Ultralight.lib)
target_link_libraries(TheCommitCommissar UltralightCore.lib)
target_link_libraries(TheCommitCommissar WebCore.lib)
target_link_libraries(TheCommitCommissar AppCore.lib)
target_link_libraries(TheCommitCommissar Common.lib)
target_link_libraries(TheCommitCommissar GraphicAPIBroker.lib)
target_link_libraries(TheCommitCommissar WolfEngine.lib)

add_dependencies(TheCommitCommissar WolfEngine)

set_target_properties(TheCommitCommissar
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../x64/${CMAKE_BUILD_TYPE}/exe"
        RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_SOURCE_DIR}/../x64/Debug/exe"
        RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}/../x64/Release/exe")