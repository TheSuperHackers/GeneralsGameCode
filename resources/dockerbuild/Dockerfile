FROM debian:12.10-slim

# Build arguments
ARG CMAKE_VERSION="3.31.6"
ARG GIT_VERSION="2.49.0"
ARG UID
ARG GID

# Install utils
RUN apt update \
    && apt install wget gpg unzip git -y \
    && dpkg --add-architecture i386 \
    && apt clean \
    && rm -rf /var/lib/apt/lists/*

# Install wine
RUN mkdir -pm755 /etc/apt/keyrings \
    && (wget -O - https://dl.winehq.org/wine-builds/winehq.key | gpg --dearmor -o /etc/apt/keyrings/winehq-archive.key -) \
    && wget -NP /etc/apt/sources.list.d/ https://dl.winehq.org/wine-builds/debian/dists/bookworm/winehq-bookworm.sources \
    && apt update \
    && apt install --no-install-recommends winehq-stable -y \
    && apt clean \
    && rm -rf /var/lib/apt/lists/*
# Setup build folder 
RUN mkdir /build
RUN chown  ${UID}:${GID} /build

USER ${UID}:${GID}
WORKDIR /build/tools

# Install cmake windows
RUN wget https://github.com/Kitware/CMake/releases/download/v${CMAKE_VERSION}/cmake-${CMAKE_VERSION}-windows-x86_64.zip \
    && unzip cmake-${CMAKE_VERSION}-windows-x86_64.zip -d /build/tools/ \
    && mv /build/tools/cmake-${CMAKE_VERSION}-windows-x86_64 /build/tools/cmake \
    && find /build/tools/ -name "*.zip" -exec rm -f {} +

# Install git windows
RUN wget https://github.com/git-for-windows/git/releases/download/v${GIT_VERSION}.windows.1/MinGit-${GIT_VERSION}-64-bit.zip \
    && unzip MinGit-${GIT_VERSION}-64-bit.zip -d /build/tools/ \
    && mv /build/tools/cmd /build/tools/git \
    && find /build/tools/ -name "*.zip" -exec rm -f {} +

# Install Visual Studio 6 Portable
RUN wget https://github.com/itsmattkc/MSVC600/archive/refs/heads/master.zip \
    && unzip master.zip -d /build/tools \
    && mv /build/tools/MSVC600-master/ /build/tools/vs6 \
    && find /build/tools/ -name "*.zip" -exec rm -f {} +


#Install ninja  
RUN wget https://github.com/ninja-build/ninja/releases/download/v1.13.1/ninja-win.zip \
    && unzip ninja-win.zip -d /build/tools/ \
    && find /build/tools/ -name "*.zip" -exec rm -f {} +

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
ENV PRESET=vc6

#prevent cache warning 
ENV HOME=/build/tmp/home
RUN mkdir /build/tmp/home

WORKDIR /build/cnc

USER root
COPY entrypoint.sh /entrypoint.sh
RUN chmod +x /entrypoint.sh
CMD /entrypoint.sh
