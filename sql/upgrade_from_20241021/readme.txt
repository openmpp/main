Upgrade models SQLite database files
from versions [2023-02-01, 2024-10-21] a.k.a. [v1.14.0, v1.17.5]
to the next schema version


IMPORTANT
=========
1. Make a backup copy of your model.sqlite database files before upgrade.
2. If your database schema is older then 2023-02-01 release then apply previous updates first.

Windows: how to run upgrade script
----------------------------------

For example,
  if new version of openM++ is in:
C:\openmpp_win_next\
                    bin\
                    sql\upgrade_from_20241021\
                                              upgrade_win.bat
						 
  and previous version of openM++ is in:
C:\openmpp_win_20241021\
                        bin\
                        models\

then:

option 1. 
  copy directory \upgrade_from_20241021\ into old openM++: C:\openmpp_win_20241021\
  double click on C:\openmpp_win_20241021\upgrade_from_20241021\upgrade_win.bat

option 2. 
  open command prompt (not PowerShell) and type:
    cd /D C:\openmpp_win_next\sql\upgrade_from_20241021
    upgrade_win.bat C:\openmpp_win_20241021


Linux or MacOS: how to run upgrade script
-----------------------------------------

For example,
  if new version of openM++ is in:
~/openmpp_debian_next/
                     bin/
                     sql/upgrade_from_20241021/
                                              upgrade_linux.sh

  and previous version of openM++ is in:
~/openmpp_debian_20241021/
                         bin/
                         models/

then:

option 1.
 cp -r ~/openmpp_debian_next/sql/upgrade_from_20241021/ ~/openmpp_debian_20241021/
 cd ~/openmpp_debian_20241021
 ./upgrade_from_20241021/upgrade_linux.sh

option 2.
 cd ~/openmpp_debian_next/sql/upgrade_from_20241021/
 ./upgrade_linux.sh ~/openmpp_debian_20241021

