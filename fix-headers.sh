#! /bin/sh

find . -type f -exec sed -i "s/<mirandaGenerator.h>/\"..\/mirandaGenerator.h\"/g" {} \;
sed -i "s/<generators\/gupsgen.h>/\"gupsgen.h\"/g" generators/gupsgen.cc
sed -i "s/<generators\/singlestream.h>/\"singlestream.h\"/g" generators/singlestream.cc
sed -i "s/<generators\/stencil3dbench.h>/\"stencil3dbench.h\"/g" generators/stencil3dbench.cc
sed -i "s/<generators\/streambench.h>/\"streambench.h\"/g" generators/streambench.cc
sed -i "s/<generators\/streambench_customcmd.h>/\"streambench_customcmd.h\"/g" generators/streambench_customcmd.cc
sed -i "s/<generators\/randomgen.h>/\"randomgen.h\"/g" generators/randomgen.cc
sed -i "s/<generators\/revsinglestream.h>/\"revsinglestream.h\"/g" generators/revsinglestream.cc
