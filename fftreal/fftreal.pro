#include(../../spectrum.pri)

#static: error(This library cannot be built for static linkage)

TEMPLATE = lib
TARGET   = fftreal
CONFIG -= qt

win32 {
CONFIG += dll
}
# FFTReal
HEADERS  += Array.h \
            Array.hpp \
            DynArray.h \
            DynArray.hpp \
            FFTRealFixLen.h \
            FFTRealFixLen.hpp \
            FFTRealFixLenParam.h \
            FFTRealPassDirect.h \
            FFTRealPassDirect.hpp \
            FFTRealPassInverse.h \
            FFTRealPassInverse.hpp \
            FFTRealSelect.h \
            FFTRealSelect.hpp \
            FFTRealUseTrigo.h \
            FFTRealUseTrigo.hpp \
            OscSinCos.h \
            OscSinCos.hpp \
            def.h
	    
# Wrapper used to export the required instantiation of the FFTRealFixLen template
HEADERS  += fftreal_wrapper.h
SOURCES  += fftreal_wrapper.cpp

DEFINES  += FFTREAL_LIBRARY

macx {
    CONFIG += lib_bundle
} else {
    DESTDIR = ../..$${spectrum_build_dir}
}

#EXAMPLE_FILES = bwins/fftreal.def eabi/fftreal.def readme.txt license.txt

#target.path = $$[QT_INSTALL_EXAMPLES]/multimedia/spectrum
#INSTALLS += target

#CONFIG += install_ok  # Do not cargo-cult this!

#EXAMPLE_FILES = bwins/fftreal.def eabi/fftreal.def readme.txt license.txt

#target.path = $$[QT_INSTALL_EXAMPLES]/multimedia/spectrum
#INSTALLS += target

#CONFIG += install_ok  # Do not cargo-cult this!

DESTDIR = $$PWD/../bin

!isEmpty(target.path): INSTALLS += target
