This do-nothing file exists just to ensure that the containing microdata directory exists in the ompp git repository.
If that directory does not exist, OzProjX may fail if MicroDataOutputFlag is ste to true,
if it's unable to open the output microdata file because the directory does not exist.