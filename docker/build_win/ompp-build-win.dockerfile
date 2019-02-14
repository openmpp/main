# escape=`

# Recommended tag name for that image: ompp-build-win
# Based on image: ompp-cpp-win
#
# Example of build:
#   docker build --isolation process --tag ompp-build-win -f ompp-build-win.dockerfile .
#
# Example of run:
#   docker run --isolation process -v C:\my\build:C:\build     ompp-build-win do-build cpp
#   docker run --isolation process -v C:\my\build:C:\build     ompp-build-win do-build tools
#   docker run --isolation process -v C:\my\build:C:\build -it ompp-build-win cmd
#   docker run --isolation process -v C:\my\build:C:\build -it ompp-build-win do-build perl
# 

FROM ompp-cpp-win

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

# install cpan pp module
RUN C:\perl\portableshell /SETENV && `
    cpanm pp

# copy build scripts
COPY do-build.bat do_build_tools.bat prompt_all.txt C:\build_scripts\

# set Go, MinGW and R environment, set portable Perl directory
USER ContainerAdministrator

RUN setx /M GOROOT C:\go
RUN setx /M GOPATH C:\build\ompp\ompp-go
RUN setx /M PERLROOT C:\perl
RUN setx /M PATH "%GOROOT%\bin;%GOPATH%\bin;C:\svn\bin;C:\R\bin;%PATH%"

USER ContainerUser

# describe image
#
LABEL name=ompp-build-win
LABEL os=Windows
LABEL license=MIT
LABEL description="OpenM++ full build environemnt: VC++ 2017, MSBuild, MS MPI, svn, Go, MinGW, R, node.js, Perl"

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
    if not exist do-build.bat (echo copy do-build.bat & copy C:\build_scripts\do-build.bat do-build.bat)) &&

CMD type C:\build_scripts\prompt_all.txt
