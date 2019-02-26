# escape=`

# Recommended tag name for that image: ompp-build-win
#
# Example of build:
#   docker build --isolation process -t ompp-build-win -t ompp-build:windows-1809 .
#
# Example of run:
#   docker run --isolation process -v C:\my\build:C:\build     ompp-build-win build-all
#   docker run --isolation process -v C:\my\build:C:\build     ompp-build-win build-openmpp
#   docker run --isolation process -v C:\my\build:C:\build     ompp-build-win build-models
#   docker run --isolation process -v C:\my\build:C:\build     ompp-build-win build-go
#   docker run --isolation process -v C:\my\build:C:\build     ompp-build-win build-r
#   docker run --isolation process -v C:\my\build:C:\build     ompp-build-win build-perl
#   docker run --isolation process -v C:\my\build:C:\build     ompp-build-win build-ui
#   docker run --isolation process -v C:\my\build:C:\build     ompp-build-win build-zip
#   docker run --isolation process -v C:\my\build:C:\build -it ompp-build-win cmd
#   docker run --isolation process -v C:\my\build:C:\build -it ompp-build-win %PERLROOT%\portableshell
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

# download and install Go and MinGW
RUN curl -L -o C:\Temp\go_setup.zip https://dl.google.com/go/go1.11.5.windows-amd64.zip && `
    C:\7zip\7z.exe x -oC:\ C:\Temp\go_setup.zip && `
    del C:\Temp\go_setup.zip

RUN curl -L -o C:\Temp\mingw_setup.exe https://nuwen.net/files/mingw/mingw-16.1-without-git.exe && `
    C:\7zip\7z.exe x -oC:\ C:\Temp\mingw_setup.exe && `
    del C:\Temp\mingw_setup.exe

# download and install R
RUN curl -L -o C:\Temp\r_setup.exe https://cloud.r-project.org/bin/windows/base/old/3.5.2/R-3.5.2-win.exe && `
    C:\Temp\r_setup.exe /SILENT /SUPPRESSMSGBOXES /DIR=C:\R /LOG && `
    del C:\Temp\r_setup.exe

# download and install node.js
RUN curl -L -o C:\Temp\node.zip https://nodejs.org/dist/v10.15.1/node-v10.15.1-win-x64.zip && `
    C:\7zip\7z.exe x -oC:\ C:\Temp\node.zip && `
    rename C:\node-v10.15.1-win-x64 node && `
    del C:\Temp\node.zip

# download and install svn command line tools
RUN curl -L -o C:\Temp\svn.zip https://www.visualsvn.com/files/Apache-Subversion-1.11.1.zip && `
    C:\7zip\7z.exe x -oC:\svn C:\Temp\svn.zip && `
    del C:\Temp\svn.zip

# download and install portable Perl
RUN curl -L -o C:\Temp\perl_setup.zip http://strawberryperl.com/download/5.28.1.1/strawberry-perl-5.28.1.1-64bit-portable.zip && `
    C:\7zip\7z.exe x -oC:\perl C:\Temp\perl_setup.zip && `
    del C:\Temp\perl_setup.zip

# install cpan modules
RUN C:\perl\portableshell /SETENV && `
    cpanm pp && `
    cpanm Getopt::Long::Descriptive

# copy build scripts
COPY copy_scripts.bat `
    build-all.bat `
    build-openm.bat `
    build-models.bat `
    build-go.bat `
    build-ui.bat `
    build-r.bat `
    build-perl.bat `
    build-zip.bat `
    prompt_build.txt C:\build_scripts\

# set Go, MinGW and R environment, set portable Perl directory
USER ContainerAdministrator

RUN setx /M GOROOT C:\go
RUN setx /M GOPATH C:\build\ompp\ompp-go
RUN setx /M PERLROOT C:\perl
RUN setx /M OM_SCRIPTS_DIR C:\build_scripts
RUN setx /M PATH "%GOROOT%\bin;%GOPATH%\bin;C:\Git\cmd;C:\svn\bin;C:\R\bin;%PATH%"

USER ContainerUser

# describe image
#
LABEL name=ompp-build-win
LABEL os=Windows
LABEL license=MIT
LABEL description="OpenM++ build environemnt: VC++ 2017, MSBuild, MS MPI, git, svn, SQLite, bison, flex, Go, MinGW, R, node.js, Perl"

# Done with installation
# 
WORKDIR /build

# start VC++ developer command prompt
# and MinGW environment with any other commands specified.
#
# known issue:
#   "Invalid macro definition." 
#   this error messages can be ignored,
#   doskey is broken in MS windows/servercore
#
ENTRYPOINT (C:\BuildTools\Common7\Tools\VsDevCmd.bat && `
    C:\MinGW\set_distro_paths.bat && `
    C:\node\nodevars.bat && `
    %OM_SCRIPTS_DIR%\copy_scripts.bat) &&

CMD type %OM_SCRIPTS_DIR%\prompt_build.txt
