How to build openM++ model using Xcode
======================================

0. Prerequisits
---------------

Install command line developer tools, if not installed already by Xcode:

xcode-select --install

-------------------------------------
1. Download latest release of openM++
-------------------------------------

Download and unpack latest openM++ release from GitHub:

https://github.com/openmpp/main/releases/latest

For example:

mkdir ~/any/dir
cd ~/any/dir
curl -L -o openmpp_mac.tar.gz https://github.com/openmpp/main/releases/download/v1.6.0/openmpp_mac_20200618.tar.gz
tar xzf openmpp_mac.tar.gz

--------------------------
2. Build model using Xcode
--------------------------

1. Copy Model Xcode project files into model directory, for example NewCaseBased model:

cd ~/any/dir/openmpp_mac
cp -pr Xcode/* models/NewCaseBased/

2. Follow screenshots on GitHub: https/github.com/opempp/mac/pictures:

2.1.Start Xcode and open ~/any/dir/openmpp_mac/models/NewCaseBased/Model.xcworkspace

2.2.Rename model
  Click on model project -> Targets -> double click taregt name -> rename to model name
  Product -> Scheme -> rename "Model" to actual model name: NewCaseBased

2.3.Add model source code from "code" folder.

2.4.Select Product -> Scheme -> NewCaseBased
  Known issue: 
  Xcode UI may not update check mark on selected scheme
  To fix it go to Product -> Scheme -> Manage Schemes
  and use mouse to drag any scheme to move it up or down.

2.5.Build and debug

===========================================
How to build openM++ from source code Xcode
===========================================

0. Prerequisits
---------------

Install command line developer tools, if not installed already by Xcode:

xcode-select --install

# install HomeBrew from GUI terminal:

/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install.sh)"

# install bison 2.7 using HomeBrew:

brew install bison@2.7

# export bison 2.7
# (optional) add to your .zprofile and logout/login

export PATH="/usr/local/opt/bison@2.7/bin:${PATH}"
export LDFLAGS="-L/usr/local/opt/bison@2.7/lib ${LDFLAGS}"

# verify bison version

bison --version

#  output:
#  bison (GNU Bison) 2.7.12-4996

--------------------------------------------
1. Clone openM++ main repository from GitHub
--------------------------------------------

git clone https://github.com/openmpp/main.git ompp-main

------------------------
2. Build omc using Xcode
------------------------

1.Start Xcode and open ~/ompp-main/openm/openm.xcworkspace

2.Select Product -> Scheme -> omc
  Known issue: 
  Xcode UI does not update check mark on selected scheme
  To fix it go to Product -> Scheme -> Manage Schemes
  and use mouse to drag any scheme to move it up or down.

3.Build and debug

4.Build release version of omc in order to compile models
  Select Product -> Build For -> Profiling

Release version of omc is required in order to build any model other than modelOne.

-----------------------------
3. Build modelOne using Xcode
-----------------------------

If you need to build modelOne for you own pleasure then
open ~/ompp-main/models/modelOne/modelOne.xcworkspace
to build and debug modelOne.
