cmake_minimum_required(VERSION 3.31)
project(TheCommitCommissar)

set(CMAKE_CXX_STANDARD 23)

file(GLOB SRC
        "*.cpp"
)

# Includes Wolf libs
include_directories("dependencies/Wolf Engine 2.0/Common")
include_directories("dependencies/Wolf Engine 2.0/GraphicAPIBroker/Public")
include_directories("dependencies/Wolf Engine 2.0/Wolf Engine 2.0")

# Includes third parties
include_directories("dependencies/Wolf Engine 2.0/ThirdParty/xxh64")
include_directories("dependencies/Wolf Engine 2.0/ThirdParty/glm")
include_directories("dependencies/Wolf Engine 2.0/ThirdParty/GLFW/include")
include_directories("dependencies/Wolf Engine 2.0/ThirdParty/tiny_obj")
include_directories("dependencies/Wolf Engine 2.0/ThirdParty/FrustumCull")
include_directories("dependencies/Wolf Engine 2.0/ThirdParty/stb_image")
include_directories("dependencies/Wolf Engine 2.0/ThirdParty/UltraLight/include")
include_directories("dependencies/Wolf Engine 2.0/ThirdParty/vulkan/Include")
include_directories("dependencies/Wolf Engine 2.0/ThirdParty/Tracy/tracy")

link_directories("dependencies/Wolf Engine 2.0/x64/Debug/lib")
link_directories("dependencies/Wolf Engine 2.0/ThirdParty/vulkan/Lib")
link_directories("dependencies/Wolf Engine 2.0/ThirdParty/GLFW/lib-vc2019")
link_directories("dependencies/Wolf Engine 2.0/ThirdParty/UltraLight/lib")
link_directories("../x64/Debug")

add_subdirectory(dependencies/traypp EXCLUDE_FROM_ALL)
link_libraries(tray)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(TheCommitCommissar ${SRC})

target_compile_definitions(TheCommitCommissar PUBLIC WOLF_VULKAN)
target_compile_definitions(TheCommitCommissar PUBLIC MATERIAL_DEBUG)
target_compile_definitions(TheCommitCommissar PUBLIC TRACY_ENABLE)

target_link_libraries(TheCommitCommissar vulkan-1.lib)
target_link_libraries(TheCommitCommissar glfw3.lib)
target_link_libraries(TheCommitCommissar Ultralight.lib)
target_link_libraries(TheCommitCommissar UltralightCore.lib)
target_link_libraries(TheCommitCommissar WebCore.lib)
target_link_libraries(TheCommitCommissar AppCore.lib)
target_link_libraries(TheCommitCommissar Common.lib)
target_link_libraries(TheCommitCommissar GraphicAPIBroker.lib)
target_link_libraries(TheCommitCommissar WolfEngine.lib)

set_target_properties(TheCommitCommissar
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../x64/Debug/exe")