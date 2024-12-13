#!/usr/bin/env bash
#
# upgrade models SQLite database files
# from versions [2023-02-01, 2024-10-21] a.k.a. [v1.14.0, v1.17.5]
#
# IMPORTANT:
# Make a backup copy of your model.sqlite database files before upgrade
#
# How to run upgrade script:
# For example,
#   if new version of openM++ is in:
# ~/openmpp_debian_next/
#                      bin/
#                      sql/upgrade_from_20241021/
#                                               upgrade_linux.sh
#
#   and previous version of openM++ is in:
# ~/openmpp_debian_20241021/
#                          bin/
#                          models/
#
# then:
#
# option 1.
#  cp -r ~/openmpp_debian_next/sql/upgrade_from_20241021/ ~/openmpp_debian_20241021/
#  cd ~/openmpp_debian_20241021
#  ./upgrade_from_20241021/upgrade_linux.sh
#
# option 2.
#  cd ~/openmpp_debian_next/sql/upgrade_from_20241021/
#  ./upgrade_linux.sh ~/openmpp_debian_20241021
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

upgrade_sql=$self_dir/upgrade_from_20241021.sql

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

# check: do not run upgrade script on the versions after 20241021

if [ -e $dst_root/sql/upgrade_from_20241021/$self ]; then
  echo "ERROR: it is look like destination folder is incorrect: $dst_root"
  echo "ERROR: it must contain versions [2023-02-01, 2024-10-21] a.k.a. [v1.14.0, v1.17.5]"
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
