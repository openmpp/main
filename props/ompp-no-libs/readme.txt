2021-03-25:

Modified version of model project to build all from c++ sources
instead of using libsqlite.lib and libopenm.lib

To use this for your model build copy
Model.vcxproj and Model.vcxproj.filters into openM++ model project directory.
For example, if you have openM++ at C:\openmpp and your model name is MyModel
then copy:
Model.vcxproj         => C:\openmpp\models\MyModel\ompp\
Model.vcxproj.filters => C:\openmpp\models\MyModel\ompp\
