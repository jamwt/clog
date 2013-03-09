clog
====

`clog` writes rotating log files, like multilog.

It reads stdin and writes to a rotating series of logs in a
directory.

Rotation Scheme
---------------

The current log file is always called "current"; previous log
files are renamed to roughly ISO 8601 format using the system
time (*hopefully* UTC).  A rotated file might be named:

    a2013-03-09_19:50:33.748839

Deviations are:

 * Files start with 'a' for easy glob matching (think: "archive").
 * Underscore is used to separate date from time
   instead of "T" for readability.
 * Zero-padded microseconds is appended after a dot.

This format keeps all files sorted in lexicographical order.
`ls LOG_DIR | tail` will always give you the last 10 files.
Similarly, `ls | head -n -5 | xargs rm` will make sure only
the most recent 5 files are in `LOG_DIR`.

Once an `a*` file is created, is is never renamed or deleted
unless you do so.  This is a useful property for various
processing schemes.

Termination
-----------

`clog` will exit and flush any buffered data into a log file
when stdin is closed.  If stdin does not end with a newline, 
clog will append a newline.

Installing
----------

    make && sudo make install

Usage
-----

Using `clog` is very simple:

    clog FILE_SIZE LOG_DIR

`clog` will make a decent effort to rotate the "current" file
when it reaches `FILE_SIZE` bytes.  `FILE_SIZE` can also contain
a suffix (K, M, G, or T), for usage like:

    /sbin/my_daemon | clog 100M /var/log/my_daemon

This would rotate files at 100 MB.  You get the idea.

Portability
-----------

Thus far, `clog` has only been built on Linux.  If you get it built
somewhere else and have patches, send 'em along.

Differences from multilog
-------------------------

 * `clog` does not support dumping timestamps in your files.
 * `clog` also does not support removing old files.
 * `clog` has no pattern matching.
