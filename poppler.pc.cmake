prefix=@CMAKE_INSTALL_PREFIX@
exec_prefix=${prefix}
libdir=${exec_prefix}/lib
includedir=${prefix}/include

Name: poppler
Description: PDF rendering library
Version: @POPPLER_VERSION@

Libs: -L${libdir} -lpoppler
Cflags: -I${includedir}/poppler
