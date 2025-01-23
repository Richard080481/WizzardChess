pushd %~dp0
%VULKAN_SDK%\bin\glslc.exe shader.frag -o frag.spv
%VULKAN_SDK%\bin\glslc.exe shader.vert -o vert.spv
