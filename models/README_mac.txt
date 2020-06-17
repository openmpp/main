Build and debug model using Xcode
---------------------------------

1.
Follow screenshots on GitHub: https/github.com/opempp/mac/pictures
Specially do not forget to set all *.mpp and *.ompp as C++ source files 

1.1. Start Xcode and open ~/any/dir/openmpp_mac/models/NewCaseBased/Model.xcworkspace

1.2. Select Product -> Scheme -> NewCaseBased
  Known issue: 
  Xcode UI may not update check mark on selected scheme
  To fix it go to Product -> Scheme -> Manage Schemes
  and use mouse to drag any scheme to move it up or down.

1.3. Build and debug


2.
In order to run OzProj or OzProjGen you need to define environment variable:

OM_ROOT=$(SRCROOT)/../..

Use menu:
  Product -> Scheme -> OzProj -> Edit Scheme... -> Run -> Environment Variables

