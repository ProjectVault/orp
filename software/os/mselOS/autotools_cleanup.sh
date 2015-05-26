rm -rf aclocal.m4 autom4te.cache config.h.in configure 

find . -name Makefile.in -delete
find . -name .dirstamp -delete
find . -name "*.o"  -delete
find . -name "*.lo" -delete
find . -name "*.a"  -delete
find . -name "*.la" -delete

rm -rf ar-lib compile config.guess config.sub depcomp install-sh ltmain.sh missing
rm -f config.h config.log config.status libtool stamp-h1
rm -f Makefile
find . -type d -name .deps -exec rm -rf {} +
