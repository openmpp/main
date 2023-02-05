Upgrade models SQLite database files
to the new schema of release 2023-02-01 a.k.a. v1.14.0
from version 2022-09-23 a.k.a. v1.11.1 or older


IMPORTANT
=========
Make a backup copy of your model.sqlite database files before upgrade.

Windows: how to run upgrade script
----------------------------------

For example,
  if new version of openM++ is in:

C:\openmpp_win_20230201\
                       bin\
                       sql\upgrade_to_20230201\
                                              upgrade_win.bat

  and previous version of openM++ is in:

C:\openmpp_win_20220923\
                       bin\
                       models\

then:

option 1.
  copy directory \upgrade_to_20230201\ into old openM++: C:\openmpp_win_20220923\
  double click on C:\openmpp_win_20220923\upgrade_to_20230201\upgrade_win.bat

option 2.
  open command prompt (not PowerShell) and type:
    cd /D C:\openmpp_win_20230201\sql\upgrade_to_20230201
    upgrade_win.bat C:\openmpp_win_20220923


Linux or MacOS: how to run upgrade script
-----------------------------------------

For example,
  if new version of openM++ is in:

~/openmpp_debian_20230201/
                       bin/
                       sql/upgrade_to_20230201/
                                              upgrade_linux.sh

  and previous version of openM++ is in:

~/openmpp_debian_20220923/
                       bin/
                       models/

then:

option 1.
 cp -r ~/openmpp_debian_20230201/sql/upgrade_to_20230201/ ~/openmpp_debian_20220923/
 cd ~/openmpp_debian_20220923
 ./upgrade_to_20230201/upgrade_linux.sh

option 2.
 cd ~/openmpp_debian_20230201/sql/upgrade_to_20230201/
 ./upgrade_to_20230201/upgrade_linux.sh ~/openmpp_debian_20220923

