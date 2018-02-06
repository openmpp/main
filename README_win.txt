Welcome to OpenM++ for the Windows platform!

If you are reading this file, you have successfully extracted the Windows OpenM++ distribution
to a directory of your chosing.  In what follows, that directory is assumed to be C:\OpenM++.
In subsequent instructions, replace C:\OpenM++ with the directory to which you extracted the
downloaded OpenM++ zip archive.

========================================
Starting OpenM++ user interface (ompp-ui)
========================================

You can start OpenM++ UI by double clicking on C:\OpenM++\bin\ompp_ui.bat.
As result you will see:
- OpenM++ web-service (oms) in background windows
- web UI in your default browser at http://localhost:4040

It is recommended to use Chrome, Firefox or Edge for ompp-ui.
Please note Internet Explorer is significantly slower and may not work correctly.

========================================
Setting the OM_ROOT environment variable
========================================

To build and develop models outside of the 'models' sub-folder of the installation directory,
you need to set the environment variable OM_ROOT persistently to the directory where you
extracted OpenM++.  One way is to open a privileged command window and type:

setx OM_ROOT C:\OpenM++

Another way is to right-click the computer icon in Windows Explorer, and then drill through 
various options to find the dialogue to modify environment variables.

========================================
Verifying the OpenM++ installation
========================================

OpenM++ model development requires at least Visual Studio 2015.  Here are steps to verify the
OpenM++ installation using the RiskPaths model, which is shipped with the OpenM++ installation.

1. Double-click the solution file C:\OpenM++\models\RiskPaths\RiskPaths-ompp.sln to open the solution
   in Visual Studio 2015.
2. If desired, use the drop-down menus in the toolbar to change the Solution Configuration
   (Debug or Release) or the Solution Platform (Win32 or x64)
3. Do Build > Rebuild Solution
4. Visual Studio will build the model, publish the Default scenario, run the model with the Default
   scenario, and export the output tables in both Excel and CSV formats.
5. After the build completes, the Output window in Visual Studio should contain a line which says
   "Build succeeded."
6. The exported model results of the Default scenario in Excel are in the workbook
   C:\OpenM++\models\RiskPaths\output\Default(tbl).xlsx
   The exported results in CSV format of hte Default scenario are in CSV files in the folder
   C:\OpenM++\models\RiskPaths\output\Default, one for each model table.
7. For more information on how to create and run models and scenarios, and visualize results
   using the PivotMaker2 application, please see the OpenM++ wiki.

==================================================
X-compatible OpenM++ / Modgen development
==================================================

This version of OpenM++ supports cross-compatible model development with either Modgen 11
or Modgen 12 (or both).  A single code base in a shared location can be used to build both
OpenM++ and Modgen versions of a model.  A directory with input parameters for a scenario
can be used to run the scenario using either the OpenM++ or Modgen version of the model.

==================================================
Verifying the x-compatible Modgen 11 installation
==================================================

Modgen 11 requires Visual Studio 2010 Professional (or above).
Here are steps to verify the cross-compatible installation for Modgen 11:

1. Open Visual Studio 2010.  Do File > Open > Project/Solution, and navigate to the solution file
   C:\OpenM++\models\RiskPaths\RiskPaths-modgen.sln.
2. Verify the drop-down menus in the toolbar and ensure that the Solution Platform is set to Win32.
   The solution platform x64 is not supported by Modgen 11.
3. Do Build > Rebuild Solution
4. Visual Studio will build the model, publish the Default scenario, run the model with the Default
   scenario, and create an MS Access database with the results.
5. After the build completes, the Output window in Visual Studio should contain a line which says
   "Build succeeded."
6. The Modgen input scenario file is located at
   C:\OpenM++\models\RiskPaths\output-modgen\Default.scex
   and the MS Access output database at
   C:\OpenM++\models\RiskPaths\output-modgen\Default(tbl).mdb
7. To examine model outputs using the Modgen user interface, open the model from Visual Studio
   using Debug > Start without debugging (or just do Ctrl-F5).  Visual Studio says the project
   is out of date, and offers to rebuild it.  Respond either Yes or No.  The model will open.
   From the Modgen interface, do Scenario > Open and navigate to the scenario input file Default.scex.
   
==================================================
Verifying the x-compatible Modgen 12 installation
==================================================

Modgen 12 requires Visual Studio 2013 Professional (or above).
Here are steps to verify the cross-compatible installation for Modgen 12:

1. Open Visual Studio 2013.  Do File > Open > Project/Solution, and navigate to the solution file
   C:\OpenM++\models\RiskPaths\RiskPaths-modgen.sln.
2. Verify the drop-down menus in the toolbar and ensure that the Solution Platform is set to Win32.
   The solution platform x64 is supported by Modgen 12, but requires special configuration.
3. Do Build > Rebuild Solution
4. Visual Studio will build the model, publish the Default scenario, run the model with the Default
   scenario, and create an MS Access database with the results.
5. After the build completes, the Output window in Visual Studio should contain a line which says
   "Build succeeded."
6. The Modgen input scenario file is located at
   C:\OpenM++\models\RiskPaths\output-modgen\Default.scex
   and the MS Access output database at
   C:\OpenM++\models\RiskPaths\output-modgen\Default(tbl).mdb
7. To examine model outputs using the Modgen user interface, open the model from Visual Studio
   using Debug > Start without debugging (or just do Ctrl-F5).  Visual Studio says the project
   is out of date, and offers to rebuild it.  Respond either Yes or No.  The model will open.
   From the Modgen interface, do Scenario > Open and navigate to the scenario input file Default.scex.
  
==================================================
Scenarios and execution options in OpenM++
==================================================

The OpenM++ build environment in Visual Studio OpenM++ supports the execution of multiple scenarios.
The inputs for each scenario are contained in a sub-directory with the scenario name, located in the
parameters sub-directory of the model.

AN OpenM++ model can be run with various options, including multi-threading.  These options can be viewed
and changed in Visual Studio from the Property Manager window, by expanding any of the build
configuration nodes, opening the 'model' property sheet, and then selecting Common properties > User Macros.
For more information, please see the OpenM++ wiki.
  
==================================================
Grid computing with OpenM++ (Windows)
==================================================
   
OpenM++ can run simulations on a grid of computers using a technology called MPI.
The grid can consist of several computers or even just one, or many hundreds of computers.
This can speed up execution by using the computational cores on multiple machines.
For more information, please see the OpenM++ wiki.
