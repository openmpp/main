#!/usr/bin/env bash
#
# upgrade models SQLite database files to release 2023-02-01 a.k.a. v1.14.0
# from version 2022-09-23 a.k.a. v1.11.1 or older
#
# IMPORTANT:
# Make a backup copy of your model.sqlite database files before upgrade
#
# How to run upgrade script:
# For example,
#   if new version of openM++ is in:
# ~/openmpp_debian_20230201/
#                        bin/
#                        sql/upgrade_to_20230201/
#                                               upgrade.sh
#
#   and previous version of openM++ is in:
# ~/openmpp_debian_20220923/
#                        bin/
#                        models/
#
# then:
#
# option 1.
#  cp -r ~/openmpp_debian_20230201/sql/upgrade_to_20230201/ ~/openmpp_debian_20220923/
#  cd ~/openmpp_debian_20220923
#  ./upgrade_to_20230201/upgrade.sh
#
# option 2.
#  cd ~/openmpp_debian_20230201/sql/upgrade_to_20230201/
#  ./upgrade_to_20230201/upgrade.sh ~/openmpp_debian_20220923
#

set -e

# if there is command line argument it must be previous openM++ version

self="$(basename "$0")"
self_dir="$(dirname "$0")"
dst_root=

if [ -n "$1" ]; then
  dst_root=$1
else
  dst_root=$PWD/..
fi

# destination root must have \models\ subfolder
# destination must have bin\sqlite3.exe or sqlite3 must be in the PATH
# upgrade sql script must exist

models_dir=$dst_root/models

if [ ! -d $models_dir ]; then
  echo "ERROR: missing models directory: $models_dir"
  exit 1
fi
echo "Upgrade models at: $models_dir"

upgrade_sql=$self_dir/upgrade_20230201.sql

if [ ! -f $upgrade_sql ]; then
  echo "ERROR: missing upgrade script: $upgrade_sql"
  exit 1
fi
echo "Upgrade script:    $upgrade_sql"

if ! type sqlite3 >/dev/null 2>&1;
then
  echo ERROR: missing sqlite3
  exit 1
fi

# check: do not run upgrade script on the same openM++ 20230201 or older

if [ -e $dst_root/sql/upgrade_to_20230201/$self ]; then
  echo "ERROR: it is look like destination folder is incorrect: $dst_root"
  echo "ERROR: it must contain version older than 20230201 a.k.a. v1.14.0"
  exit 1
fi

# for each model.sqlite run upgrade.sql script

for f in $(find $models_dir -name '*.sqlite'); do
  if [ -f $f ]; then
    echo $f

    if ! sqlite3 $f < $upgrade_sql >/dev/null;
    then
      echo "ERROR: at sqlite3 $f < $upgrade_sql"
      exit 1
    fi
  fi
done

echo Done.
