project "magic_mem"
   kind "StaticLib"
   language "C"
   staticruntime "On"

   targetdir ("%{wks.location}/bin/" .. OutputDir .. "/%{prj.name}")
   objdir ("%{wks.location}/bin/int/" .. OutputDir .. "/%{prj.name}")

   files { "**.h", "**.c" }

   includedirs { "%{prj.location}" }