prefix=@CMAKE_INSTALL_PREFIX@
exec_prefix=${prefix}
libdir=${exec_prefix}/lib
includedir=${prefix}/include

Name: poppler-qt4
Description: Qt4 bindings for poppler
Version: @POPPLER_VERSION@
Requires: poppler = @POPPLER_VERSION@

Libs: -L${libdir} -lpoppler-qt4
Cflags: -I${includedir}/poppler/qt4
