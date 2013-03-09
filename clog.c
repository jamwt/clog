#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>

#define OUTP_LINE_BUF 2000
#define INP_BUFSIZ (256 * 1024)
static char *prog;

int64_t parse_file_size(char *s) {
    char *end = NULL;
    errno = 0;
    int64_t val = strtoll(s, &end, 10);

    if (errno)
        return -1;

    switch (*end) {
        case 0:
            return val;
        case 'k':
        case 'K':
            return val * 1024;
        case 'm':
        case 'M':
            return val * 1024 * 1024;
        case 'g':
        case 'G':
            return val * 1024 * 1024 * 1024;
        case 't':
        case 'T':
            return val * 1024 * 1024 * 1024 * 1024;
        default:
            return -1;
    }
    return -1;
}

void fatal(char *s) {
    fprintf(stderr, "error: %s\n", s);
    fprintf(stderr, "%s FILE_SIZE LOG_DIR\n", prog);
    exit(1);
}

void rotate(char *directory, char *current_file) {
    int buflen = strlen(directory) + 50;
    char *buf = alloca(strlen(directory) + 50);

    struct timeval tv;
    gettimeofday(&tv, NULL);

    snprintf(buf, buflen, "%s%d.%06d",
    directory, (int)tv.tv_sec, (int)tv.tv_usec);

    int r = rename(current_file, buf);

    if (r < 0) {
        perror("error rotating current");
        fatal("current rotation rename() error");
    }
}

int main(int argc, char **argv) {
    prog = argv[0];
    --argc;
    if (argc != 2) {
        fatal("two arguments required");
    }
    uint8_t buf[INP_BUFSIZ];

    int64_t max_size = parse_file_size(argv[1]);
    if (max_size < 0) {
        fatal("error parsing size specification");
    }
    if (max_size < 10 * 1024) {
        fatal("size specification must be at least 10k");
    }

    int d_len = strlen(argv[2]);
    char *directory = alloca(strlen(argv[2]) + 2);
    strcpy(directory, argv[2]);
    if (directory[d_len-1] != '/') {
        strcat(directory, "/");
        ++d_len;
    }

    char *current_file = alloca(d_len + 50);
    strcpy(current_file, directory);
    strcat(current_file, "current");

    int in_buffer = 0;

    while (1) {
        int fd = open(current_file, O_CREAT | O_WRONLY, 0644);
        if (fd < 0) {
            fatal("could not open file 'current' within log directory");
        }

        off_t here = lseek(fd, 0, SEEK_END);

        while (1) {

            if (here >= max_size - OUTP_LINE_BUF) {
                close(fd);
                rotate(directory, current_file);
                break;
            }

            int bread;
            do {
                errno = 0;
                bread = read(0, buf + in_buffer, INP_BUFSIZ - in_buffer);
            } while (bread < 0 && errno == EINTR);

            if (bread <= 0) {
                if (in_buffer) {
                    int w = write(fd, buf, in_buffer);
                    assert(w == in_buffer);
                    if (buf[in_buffer -1] != '\n') {
                        w = write(fd, "\n", 1);
                        assert(w == 1);
                    }
                }
                close(fd);

                if (bread < 0) {
                    perror("read error");
                    fatal("unusual error reading on stdin");
                }
                else {
                    goto out; // end of stdin
                }
            }

            int total = in_buffer + bread;

            uint8_t *p = buf + total - 1;
            uint8_t *end = p;
            for (; p >= buf; --p) {
                if (*p == '\n') {
                    end = p;
                    if (((end + 1) - buf) + here < max_size) {
                        break;
                    }
                }
            }
            ++end;

            int written = write(fd, buf, end - buf);
            here += written;
            assert(written == end - buf);
            in_buffer = total - (end - buf);
            if (in_buffer) {
                memmove(buf, end, in_buffer);
            }
        }
    }

out:
    return 0;
}
