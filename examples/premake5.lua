project "examples"
   kind "ConsoleApp"
   language "C"
   staticruntime "On"

   targetdir ("%{wks.location}/bin/" .. OutputDir .. "/%{prj.name}")
   objdir ("%{wks.location}/bin/int/" .. OutputDir .. "/%{prj.name}")

   files { "**.h", "**.c" }

   includedirs
   {
      "%{prj.location}", 
	  "%{IncludeDir.magic_mem}",
      "%{IncludeDir.clay}",
   }

   links
   {
      "magic_mem",
   }

