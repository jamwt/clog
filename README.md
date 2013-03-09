clog
====

`clog` writes rotating log files, like multilog.

It reads stdin and writes to a rotating series of logs in a
directory.

The current log file is always called "current"; previous log
files are renamed "X.Y", where X is the integer seconds since
the epoch when the file was rotated, and Y is the 0-padded
representation of the corresponding microseconds elapsed.

`clog` will exit when stdin is closed.  If stdin does not
end with a newline, clog will append a newline.

Using `clog` is very simple:

    clog FILE_SIZE LOG_DIR

Clog will make a decent effort to rotate the "current" file
when it reaches `FILE_SIZE` bytes.  `FILE_SIZE` can also contain
a prefix (k|m|g|t), for usage like:

    /sbin/my_daemon | clog 100m /var/log/my_daemon

Note: unlike multilog, `clog` does not support dump timestamps in
your files.  `clog` also does not (yet) support removing old files.
