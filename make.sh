
# make all Linux binaries

MAKE_TARGET_ARCH=X64 make clean all
MAKE_TARGET_ARCH=BBB make clean all
MAKE_TARGET_ARCH=RPI make clean all

echo "To make CYGWIN version, use Cygwin-Eclipse project manually."

