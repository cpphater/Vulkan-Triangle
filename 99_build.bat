@REM This is a build script; Get `cl` to work yourself 

@IF NOT EXIST .\build mkdir .\build
@pushd .\build

@REM /favor:<blend|AMD64|INTEL64|ATOM>
@REM         Produces code that is optimized for a specified architecture,
@REM         or for a range of architectures.
@REM /O1	    Creates small code.
@REM /O2	    Creates fast code.
@REM /Ob<n>	Controls inline expansion.
@REM /Od	    Disables optimization.
@REM /Oi[-]	Generates intrinsic functions.
@REM /Os	    Favors small code.
@REM /Ot	    Favors fast code.
@REM /Ox	    A subset of /O2 that doesn't include /GF or /Gy.
@REM /Oy	    Omits frame pointer. (x86 only)
@REM /GF	    Enables string pooling
@REM /GL	    Whole program optimization allows the compiler to perform
@REM         optimizations with information on all modules in the program.
@REM         Without whole program optimization, optimizations are performed
@REM         on a per-module (compiland) basis. /ZI can't be used with /GL
@REM         Compile with this only if you use generated exe on this machine
@REM         Isn't portable
@REM /Gw[-]  Causes the compiler to package global data in individual COMDAT sections.
@REM         When both /Gw and /GL are enabled, the linker uses whole-program optimization
@REM         to compare COMDAT sections across multiple object files in order to
@REM         exclude unreferenced global data or to merge identical
@REM         read-only global data.
@REM         This can significantly reduce the size of the resulting binary executable.
@REM /GR[-]	Enables run-time type information (RTTI).
@REM /GS[-]	Checks buffer security.
@REM /Zi   	Enables debugging

@REM /analyze           Simply talks shit about your code
@REM /JMC               When debugging with Visual Studio (at least 17), steps over 
@REM                    non-user functions  

@REM /MT                link staticly;
@REM /MD                link dll
@REM /link /opt:ref     eliminates functions and data that are never referenced
@REM /Fm<filepath>      Tells all the garbage you are linked with

@REM DEBUG:       /JMC /analyze /Od /Zi /Oi-
@REM PERFORMANCE: /O2 /Oi /GL /Gw /GR- /GS-

@set CL_FLAGS=/GR- /EHa- /JMC /analyze /Od /Zi /Oi-

@set COMMON_CL_FLAGS=/Fmmain.map /MT /nologo /Wall /WX
@set COMMON_LINK_FLAGS=/opt:ref User32.lib C:\VulkanSDK\1.3.268.0\Lib\vulkan-1.lib
@set MAIN_FILE=00_main.c


@REM 64 bit
cl %COMMON_CL_FLAGS% %CL_FLAGS% /Tc ..\%MAIN_FILE% /link %COMMON_LINK_FLAGS%

@popd
