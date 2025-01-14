project "tests"
   kind "ConsoleApp"
   language "C++"
   cppdialect "C++20"
   staticruntime "On"

   targetdir ("%{wks.location}/bin/" .. OutputDir .. "/%{prj.name}")
   objdir ("%{wks.location}/bin/int/" .. OutputDir .. "/%{prj.name}")

   files { "**.h", "**.cpp" }

   includedirs
   {
      "%{prj.location}", 
	  "%{IncludeDir.magic_arena}",
     "%{IncludeDir.spdlog}",
     "%{IncludeDir.doctest}",
   }

   links 
   {
      "magic_arena",
   }
