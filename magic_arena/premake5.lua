project "magic_arena"
   kind "StaticLib"
   language "C++"
   cppdialect "C++20"
   staticruntime "On"

   targetdir ("%{wks.location}/bin/" .. OutputDir .. "/%{prj.name}")
   objdir ("%{wks.location}/bin/int/" .. OutputDir .. "/%{prj.name}")

   pchheader "magic_pch.h"
   pchsource "magic_pch.cpp"

   files { "**.h", "**.cpp" }

   includedirs
   {
      "%{prj.location}", 
      "%{IncludeDir.spdlog}",
   }

