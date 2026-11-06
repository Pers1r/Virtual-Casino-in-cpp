cmake_minimum_required(VERSION 3.16)
project(VCasino VERSION 1.0 LANGUAGES CXX)

set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Font
file(COPY Nunito-VariableFont_wght.ttf DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include(FetchContent)
FetchContent_Declare(
        SFML
        GIT_REPOSITORY https://github.com/SFML/SFML.git
        GIT_TAG        3.0.0
)


FetchContent_MakeAvailable(SFML)

add_executable(
        VCasino 
        
        src/main.cpp
        src/player.cpp
        #src/token.cpp
        src/tokenManager.cpp

        # imgui
        imgui/imgui-SFML.cpp
        imgui/imgui.cpp
        imgui/imgui_draw.cpp
        imgui/imgui_tables.cpp
        imgui/imgui_widgets.cpp
        imgui/ImGuiFileDialog.cpp)

target_include_directories(VCasino PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/imgui
)

target_link_libraries(VCasino PRIVATE SFML::Graphics SFML::Window SFML::System)