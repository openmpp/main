2021-03-26:

Modified version of model project to build with AddressSanitizer.

To use this for your model build copy
Model.vcxproj and Model.vcxproj.filters into openM++ model project directory.
For example, if you have openM++ at C:\openmpp and your model name is MyModel
then copy:
Model.vcxproj         => C:\openmpp\models\MyModel\ompp\
Model.vcxproj.filters => C:\openmpp\models\MyModel\ompp\

You need to have AddressSanitizer installed to build model.exe

You need to have AddressSanitizer DLLs avaliable to your model.exe at run time
and options are:
- run model.exe from Visual Studio 
- run model.exe from Visual Studio Developer Command Prompt or Native Tools Command Prompt
- set environmemnt variables for model.exe to find those DLLs

amc1999: openmpp99@gmail.com
