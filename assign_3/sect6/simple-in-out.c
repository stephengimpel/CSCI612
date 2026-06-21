#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <getopt.h> /* getopt_long() */

#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h> /* low-level i/o */
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include <linux/videodev2.h>

#include <time.h>

#define CLEAR(x) memset(&(x), 0, sizeof(x))
#define COLOR_CONVERT
#define HRES 320
#define VRES 240
#define HRES_STR "320"
#define VRES_STR "240"

// Format is used by a number of functions, so made as a file global
static struct v4l2_format fmt;

struct buffer {
    void *start;
    size_t length;
};

static char *dev_name;
static int fd = -1;
struct buffer *buffers;
static unsigned int n_buffers;
static int out_buf;
static int force_format = 1;
static int frame_count = 30;

char pgm_dumpname[64];

static void ensure_dir(const char *path) {
    if (mkdir(path, 0777) == -1) {
        if (errno != EEXIST) {
            perror("mkdir");
        }
    }
}

static void dump_pgm(const void *p, int width, int height, unsigned int tag, struct timespec *time) {
    int dumpfd;
    char header[256];
    size_t size = (size_t)width * height;

    snprintf(header, sizeof(header), "P5\n# %ld sec %ld msec\n%d %d\n255\n", time->tv_sec, time->tv_nsec / 1000000, width, height);

    ensure_dir("out");
    snprintf(pgm_dumpname, sizeof(pgm_dumpname), "out/test%08d.pgm", tag);

    dumpfd = open(pgm_dumpname, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (dumpfd < 0) {
        perror("open");
        return;
    }

    write(dumpfd, header, strlen(header));
    write(dumpfd, p, size);
    close(dumpfd);
}

static int has_pgm_extension(const char *name) {
    const char *dot = strrchr(name, '.');
    return dot && (strcmp(dot, ".pgm") == 0 || strcmp(dot, ".PGM") == 0);
}

static int cmp_string_ptrs(const void *a, const void *b) {
    const char *const *sa = a;
    const char *const *sb = b;
    return strcmp(*sa, *sb);
}

static unsigned char *read_pgm(const char *filename, int *width, int *height) {
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        perror("fopen");
        return NULL;
    }

    char magic[3];
    fscanf(fp, "%2s", magic);

    if (strcmp(magic, "P5") != 0) {
        fprintf(stderr, "Not P5 format\n");
        fclose(fp);
        return NULL;
    }

    // skip comments and whitespace
    int c = fgetc(fp);
    while (c == '#') {
        while (fgetc(fp) != '\n')
            ;
        c = fgetc(fp);
    }
    ungetc(c, fp);

    int maxval;
    fscanf(fp, "%d %d", width, height);
    fscanf(fp, "%d", &maxval);
    fgetc(fp); // consume newline

    int size = (*width) * (*height);

    unsigned char *data = malloc(size);
    fread(data, 1, size, fp);

    fclose(fp);
    return data;
}

static void process_frame(unsigned char *img, int width, int height, int frame_id) {
    struct timespec t;
    clock_gettime(CLOCK_REALTIME, &t);
    dump_pgm(img, width, height, frame_id, &t);
}

static void process_input_directory(const char *dirpath) {
    DIR *dir = opendir(dirpath);
    if (!dir) {
        perror("opendir");
        exit(EXIT_FAILURE);
    }

    char **files = NULL;
    size_t count = 0, cap = 0;

    struct dirent *ent;
    while ((ent = readdir(dir)) != NULL) {
        if (ent->d_name[0] == '.')
            continue;

        if (!has_pgm_extension(ent->d_name))
            continue;

        if (count == cap) {
            cap = cap ? cap * 2 : 64;
            files = realloc(files, cap * sizeof(*files));
            if (!files) {
                perror("realloc");
                exit(EXIT_FAILURE);
            }
        }

        files[count] = malloc(strlen(dirpath) + strlen(ent->d_name) + 2);
        if (!files[count]) {
            perror("malloc");
            exit(EXIT_FAILURE);
        }

        sprintf(files[count], "%s/%s", dirpath, ent->d_name);
        count++;
    }

    closedir(dir);

    qsort(files, count, sizeof(*files), cmp_string_ptrs);

    for (size_t i = 0; i < count; i++) {
        int w, h;

        unsigned char *img = read_pgm(files[i], &w, &h);
        if (!img) continue;

        printf("Processing %s (%dx%d)\n", files[i], w, h);

        process_frame(img, w, h, i);

        free(img);
    }

    free(files);
}

static void process_single_file(const char *filepath) {
    int w, h;
    unsigned char *img = read_pgm(filepath, &w, &h);
    if (!img) return;

    printf("Processing %s (%dx%d)\n", filepath, w, h);
    process_frame(img, w, h, 0);

    free(img);
}

int main(int argc, char **argv) {
    int opt;
    const char *single_file = NULL;

    while ((opt = getopt(argc, argv, "f:")) != -1) {
        switch (opt) {
        case 'f':
            single_file = optarg;
            break;
        default:
            fprintf(stderr, "Usage: %s [-f file.pgm]\n", argv[0]);
            return EXIT_FAILURE;
        }
    }

    if (single_file) {
        process_single_file(single_file);
    } else {
        process_input_directory("./input");
    }

    return 0;
}