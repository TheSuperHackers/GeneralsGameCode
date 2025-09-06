FROM debian:12.10-slim

# Build arguments
ARG CMAKE_VERSION="3.31.6"
ARG GIT_VERSION="2.49.0"

WORKDIR /build

# Install utils
RUN apt update \
    && apt install wget gpg unzip git -y \
    && dpkg --add-architecture i386

# Install wine
RUN mkdir -pm755 /etc/apt/keyrings \
    && (wget -O - https://dl.winehq.org/wine-builds/winehq.key | gpg --dearmor -o /etc/apt/keyrings/winehq-archive.key -) \
    && wget -NP /etc/apt/sources.list.d/ https://dl.winehq.org/wine-builds/debian/dists/bookworm/winehq-bookworm.sources \
    && apt update \
    && apt install --no-install-recommends winehq-stable -y

# Clean up APT
RUN apt clean \
    && rm -rf /var/lib/apt/lists/* \

WORKDIR /build/tools/

# Install cmake windows
RUN wget https://github.com/Kitware/CMake/releases/download/v${CMAKE_VERSION}/cmake-${CMAKE_VERSION}-windows-x86_64.zip \
    && unzip cmake-${CMAKE_VERSION}-windows-x86_64.zip -d /build/tools/ \
    && mv /build/tools/cmake-${CMAKE_VERSION}-windows-x86_64 /build/tools/cmake

# Install git windows
RUN wget https://github.com/git-for-windows/git/releases/download/v${GIT_VERSION}.windows.1/MinGit-${GIT_VERSION}-64-bit.zip \
    && unzip MinGit-${GIT_VERSION}-64-bit.zip -d /build/tools/ \
    && mv /build/tools/cmd /build/tools/git

# Install Visual Studio 6 Portable
RUN wget https://github.com/itsmattkc/MSVC600/archive/refs/heads/master.zip \
    && unzip master.zip -d /build/tools \
    && mv /build/tools/MSVC600-master/ /build/tools/vs6

# Clean up downloads
RUN find /build/tools/ -name "*.zip" -exec rm -f {} +

WORKDIR /build

# Setup wine prefix
ENV WINEDEBUG=-all
ENV WINEARCH=win64
ENV WINEPREFIX=/build/prefix64

# Create empty TEMP folder for linking
RUN mkdir /build/tmp
ENV TMP="Z:\\build\\tmp"
ENV TEMP="Z:\\build\\tmp"
ENV TEMPDIR="Z:\\build\\tmp"

# Setup Visual Studio 6 Environment variables
ENV VS="Z:\\build\\tools\\vs6"
ENV MSVCDir="$VS\\vc98"
ENV WINEPATH="C:\\windows\\system32;\
$VS\\vc98\\bin;\
$VS\\vc98\\lib;\
$VS\\vc98\\include;\
$VS\\common\\Tools;\
$VS\\common\\MSDev98\\bin"
ENV LIB="$VS\\vc98\\Lib;$VS\\vc98\\MFC\\Lib;Z:\\build\\cnc\\build\\vc6"
ENV INCLUDE="$VS\\vc98\\ATL\\INCLUDE;\
$VS\\vc98\\INCLUDE;\
$VS\\vc98\\MFC\\INCLUDE;\
$VS\\vc98\\Include"
ENV CC="$VS\\vc98\\bin\\CL.exe"
ENV CXX="$VS\\vc98\\bin\\CL.exe"

WORKDIR /build/cnc

# Copy Source code
ENV GIT_VERSION_STRING="2.49.0"
# Enable if you dont build with docker compose
# COPY . .
WORKDIR /build/cnc
VOLUME /build/cnc

ENV PRESET=vc6

# Compile
CMD wineboot \
    && wine /build/tools/cmake/bin/cmake.exe \
        --preset ${PRESET} \
        -DCMAKE_SYSTEM="Windows" \
        -DCMAKE_SYSTEM_NAME="Windows" \
        -DCMAKE_SIZEOF_VOID_P=4 \
        -DCMAKE_MAKE_PROGRAM="Z:/build/tools/vs6/vc98/bin/nmake.exe" \
        -DCMAKE_C_COMPILER="Z:/build/tools/vs6/vc98/bin/cl.exe" \
        -DCMAKE_CXX_COMPILER="Z:/build/tools/vs6/vc98/bin/cl.exe" \
        -DGIT_EXECUTABLE="Z:/build/tools/git/git.exe" \
        -DCMAKE_FIND_ROOT_PATH_MODE_PROGRAM=NEVER \
        -DCMAKE_FIND_ROOT_PATH_MODE_LIBRARY=ONLY \
        -DCMAKE_FIND_ROOT_PATH_MODE_INCLUDE=ONLY \
        -DCMAKE_FIND_ROOT_PATH_MODE_PACKAGE=ONLY \
        -DCMAKE_C_COMPILER_WORKS=1 \
        -DCMAKE_CXX_COMPILER_WORKS=1 \
        -B /build/cnc/build/docker \
    && cd /build/cnc/build/docker \
    && wine cmd /c "set TMP=Z:\build\tmp& set TEMP=Z:\build\tmp& Z:\build\tools\vs6\VC98\Bin\NMAKE.exe"
