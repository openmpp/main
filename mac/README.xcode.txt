0. Prerequisits
---------------

# install command line developer tools, if not installed already

xcode-select --install

# install homebrew from GUI terminal

/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install.sh)"

# install bison 2.7 using HomeBrew

brew install bison@2.7

# export bison 2.7
# (optional) add to your .zprofile and logout/login

export PATH="/usr/local/opt/bison@2.7/bin:${PATH}"
export LDFLAGS="-L/usr/local/opt/bison@2.7/lib ${LDFLAGS}"

# verify bison version

bison --version

#  output:
#  bison (GNU Bison) 2.7.12-4996

---------------------
1. Clone openM++ main
---------------------

git clone https://github.com/openmpp/main.git ompp-main

--------------------------------
2. Add Xcode projects to openM++
--------------------------------

# clone from GitHub

cd ompp-main
git clone https://github.com/ompp/xcode.git ompp-xcode

# copy projects into our main tree

cp -pr ompp-xcode/openm/openm.xcworkspace openm/
cp -pr ompp-xcode/openm/libsqlite/* openm/libsqlite/
cp -pr ompp-xcode/openm/libopenm/* openm/libopenm/
cp -pr ompp-xcode/openm/omc/* openm/omc/

cp -p ompp-xcode/models/Model-common.xcconfig models/
cp -p ompp-xcode/models/*.sh models/

--------------------------------
3.Build omc using Xcode
--------------------------------

1.Start Xcode and open /ompp-main/openm/openm.xcworkspace

2.Select Product -> Scheme -> omc
  Known issue: 
  Xcode UI does not update check mark on selected scheme
  To fix it go to Product -> Scheme -> Manage Schemes
  and use mouse to drag any scheme to move it up or down.

3.Build and debug

4.Build release version of omc in order to compile models
  Select Product -> Build For -> Profiling

--------------------------------
4.Build NewCaseBased using Xcode
--------------------------------

1.
# copy Model Xcode project files into model directory

cp -pr ompp-xcode/models/model-xcode/* models/NewCaseBased/

2.Start Xcode and open /ompp-main/models/NewCaseBased/Model.xcworkspace

3.Rename model
Click on model project -> Targets -> double click taregt name -> rename to model name
Product -> Scheme -> rename "model" to actual model name

4.Add model source code from "code" folder.

5.Select Product -> Scheme -> model
  Known issue: 
  Xcode UI does not update check mark on selected scheme
  To fix it go to Product -> Scheme -> Manage Schemes
  and use mouse to drag any scheme to move it up or down.

6.Build and debug

----------------------------
5.Build modelOne using Xcode
----------------------------

# if you need to build modelOne then
# copy Xcode project files into model directory

cp -pr ompp-xcode/models/modelOne/* models/modelOne/
