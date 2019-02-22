# escape=`

# Recommended tag name for that image: ompp-run-win
#
# Example of build:
#   docker build --isolation process --tag ompp-run-win -f ompp-run-win.dockerfile .
#
# Example of run:
#   docker run --isolation process -v C:\my\ompp:C:\ompp -it ompp-run-win
# 

FROM mcr.microsoft.com/windows/servercore:1809

# download and install 7zip and curl
ADD https://www.7-zip.org/a/7z1806-x64.exe C:\Temp\7z_setup.exe

RUN C:\Temp\7z_setup.exe /S /D=C:\7zip\ && `
    del C:\Temp\7z_setup.exe

ADD https://curl.haxx.se/windows/dl-7.64.0/curl-7.64.0-win64-mingw.zip C:\Temp\curl.zip

RUN C:\7zip\7z.exe x -oC:\curl C:\Temp\curl.zip && `
    del C:\Temp\curl.zip

# download and install VC2015 re-distributable runtimes
RUN curl -L -o C:\Temp\vc_redist.x86.exe https://download.microsoft.com/download/6/A/A/6AA4EDFF-645B-48C5-81CC-ED5963AEAD48/vc_redist.x86.exe && `
    C:\Temp\vc_redist.x86.exe /install /quiet /norestart /log C:\Temp\log.x86.txt

RUN curl -L -o C:\Temp\vc_redist.x64.exe https://download.microsoft.com/download/6/A/A/6AA4EDFF-645B-48C5-81CC-ED5963AEAD48/vc_redist.x64.exe && `
    C:\Temp\vc_redist.x64.exe /install /quiet /norestart /log C:\Temp\log.x64.txt

# download and install MS MPI runtime
RUN curl -L -o C:\Temp\msmpisetup.exe https://github.com/Microsoft/Microsoft-MPI/releases/download/v10.0/msmpisetup.exe && `
    C:\Temp\msmpisetup.exe && `
    del C:\Temp\msmpisetup.exe

# set openM++ root directory
USER ContainerAdministrator

RUN setx /M OM_ROOT C:\ompp

USER ContainerUser

# describe image
#
LABEL name=ompp-run-win
LABEL os=Windows
LABEL license=MIT
LABEL description="OpenM++ runtime environemnt: VC++ 2015 runtime (Release only), MS MPI"

# Done with installation
# 
WORKDIR /ompp

# CMD echo OpenM++ runtime environemnt: only Release, VC++ Debug runtime not installed
