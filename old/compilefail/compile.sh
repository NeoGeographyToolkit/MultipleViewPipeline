g++  -Wall -fPIC -I/opt/local/include -I/opt/local/include/octave-3.6.3 -c blah.cpp

g++  -dynamiclib -o libzip.dylib blah.o -L/opt/local/lib/octave/3.6.3 -loctinterp -loctave -lcruft 

g++    -Wall -fPIC -o test.o -c test.cpp

# this works:
#g++   test.o  -o run -L/opt/local/lib/octave/3.6.3 libzip.dylib  /opt/local/lib/octave/3.6.3/liboctinterp.dylib -loctave -lcruft /opt/local/lib/libprotobuf.dylib 
# this doesn't work:
g++   test.o  -o run -L/opt/local/lib/octave/3.6.3 libzip.dylib /opt/local/lib/libprotobuf.dylib  /opt/local/lib/octave/3.6.3/liboctinterp.dylib -loctave -lcruft

#if you recompile protobuffers with mp-gcc45 (same as what octave was compiled with), both work
