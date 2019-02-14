# escape=`

# Recommended tag name for that image: ompp-cpp-win
#
# Example of build:
#   docker build --isolation process --tag ompp-cpp-win -f ompp-cpp-win.dockerfile .
#
# Example of run:
#   docker run --isolation process -v C:\my\build:C:\build     ompp-cpp-win do-cpp
#   docker run --isolation process -v C:\my\build:C:\build -it ompp-cpp-win cmd
# 

FROM mcr.microsoft.com/windows/servercore:1809

# download and install 7zip and curl
ADD https://www.7-zip.org/a/7z1806-x64.exe C:\Temp\7z_setup.exe

RUN C:\Temp\7z_setup.exe /S /D=C:\7zip\ && `
    del C:\Temp\7z_setup.exe

ADD https://curl.haxx.se/windows/dl-7.64.0/curl-7.64.0-win64-mingw.zip C:\Temp\curl.zip

RUN C:\7zip\7z.exe x -oC:\curl C:\Temp\curl.zip && `
    del C:\Temp\curl.zip

# download and install v15 (latest) Microsoft MSBuild Tools and VC++
RUN curl -L -o C:\Temp\vs_buildtools.exe https://aka.ms/vs/15/release/vs_buildtools.exe && `
    ( C:\Temp\vs_buildtools.exe --quiet --wait --norestart --nocache `
    --installPath C:\BuildTools `
    --add Microsoft.VisualStudio.Workload.MSBuildTools `
    --add Microsoft.VisualStudio.Workload.VCTools `
    --add Microsoft.VisualStudio.ComponentGroup.NativeDesktop.Win81 `
  || IF "%ERRORLEVEL%"=="3010" EXIT 0 ) && `
    del C:\Temp\vs_buildtools.exe

# download and install MS MPI runtime and SDK
RUN curl -L -o C:\Temp\msmpisetup.exe https://github.com/Microsoft/Microsoft-MPI/releases/download/v10.0/msmpisetup.exe && `
    C:\Temp\msmpisetup.exe && `
    del C:\Temp\msmpisetup.exe

RUN curl -L -o C:\Temp\msmpisdk.msi https://github.com/Microsoft/Microsoft-MPI/releases/download/v10.0/msmpisdk.msi && `
    msiexec /i C:\Temp\msmpisdk.msi /q /norestart /L* C:\Temp\msmpisdk.log &&`
    del C:\Temp\msmpisdk.msi && `
    del C:\Temp\msmpisdk.log

# download and install git
RUN curl -L -o C:\Temp\git_setup.exe https://github.com/git-for-windows/git/releases/download/v2.20.1.windows.1/Git-2.20.1-64-bit.exe && `
    C:\Temp\git_setup.exe /SILENT /SUPPRESSMSGBOXES /DIR=C:\Git /LOG && `
    del C:\Temp\git_setup.exe

# download and install flex and bison for Windows
# using win_flex_bison-2.4.12.zip because bison must be: 2.5 <= version < 3.0
# set environment for openM++ build
ENV BISON_FLEX_DIR=C:\bison_flex

RUN curl -L -o C:\Temp\winflexbison.zip https://github.com/lexxmark/winflexbison/releases/download/v2.4.12/win_flex_bison-2.4.12.zip && `
    C:\7zip\7z.exe x -o%BISON_FLEX_DIR% C:\Temp\winflexbison.zip && `
    del C:\Temp\winflexbison.zip
    
# download and install sqlite command line tools for Windows
# set environment for openM++ build
ENV SQLITE_EXE_DIR=C:\sqlite

RUN curl -L -o C:\Temp\sqlite_bin.zip https://www.sqlite.org/2018/sqlite-tools-win32-x86-3260000.zip && `
    C:\7zip\7z.exe e -o%SQLITE_EXE_DIR% C:\Temp\sqlite_bin.zip && `
    del C:\Temp\sqlite_bin.zip

# copy build scripts
COPY do-cpp.bat do_build_cpp.bat prompt_cpp.txt C:\build_scripts\

# add git to PATH
USER ContainerAdministrator

RUN setx /M PATH "C:\Git\cmd;%PATH%"

USER ContainerUser

# describe image
#
LABEL name=ompp-cpp-win
LABEL os=Windows
LABEL license=MIT
LABEL description="OpenM++ core build environemnt: VC++ 2017, MSBuild, MS MPI, git, bison, flex"

# Done with installation
# 
WORKDIR /build

# start VC++ developer command prompt with any other commands specified.
ENTRYPOINT (C:\BuildTools\Common7\Tools\VsDevCmd.bat && `
    if not exist do-cpp.bat (echo copy do-cpp.bat & copy C:\build_scripts\do-cpp.bat \build\do-cpp.bat)) &&

CMD type C:\build_scripts\prompt_cpp.txt
