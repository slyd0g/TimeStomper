# TimeStomper
PoC that manipulates Windows file times using SetFileTime() API

# Usage
```                 <TimeStomper Usage>

NOTE: All times are interpreted as UTC.
      Milliseconds are always random.
      Use "r" instead of <date> <time> to set random time (see example).

   -m <date> <time> (set last write time)
   -a <date> <time> (set last access time)
   -c <date> <time> (set creation time)
   -z <date> <time> (set all times)
   -p <full-path> (file or folder to set time)
   -p2 <full-path> (file or folder to copy time from)
   -r (recurse through all subfolders and files)
   -h (print this menu)

Example:
   -m r -p C:\full\path (set last modified time at C:\full\path to random date and time)
   -z 10-20-1994 14:2:01 -p C:\full\path -r (recursively set MAC time for files under C:\full\path to October 20, 1994 2:02:01 PM)
   -p C:\full\path -p2 C:\full\path2 (copies MAC time from C:\full\path2 to C:\full\path)
```

# Blogpost
TO-COME

# References
https://stackoverflow.com/questions/67273/how-do-you-iterate-through-every-file-directory-recursively-in-standard-c

https://github.com/rapid7/meterpreter/blob/master/source/extensions/priv/server/timestomp.c

https://cyberforensicator.com/2018/03/25/windows-10-time-rules/

https://www.blackbagtech.com/blog/2017/05/02/master-file-table-basics/

https://www.andreafortuna.org/cybersecurity/macb-times-in-windows-forensic-analysis/

https://whereismydata.wordpress.com/2009/02/14/dates-ntfs-created-modified-accessed-written/

https://whereismydata.wordpress.com/2008/08/11/file-system-mft-technical/

https://gallery.technet.microsoft.com/scriptcenter/Get-MFT-Timestamp-of-a-file-9227f399

https://learn-powershell.net/2013/03/03/finding-a-files-mft-timestamp-using-powershell/

https://www.forensicswiki.org/wiki/Timestomp
