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

#define TARGET_THRESHOLD 150
#define CROSSHAIR_HALF_LEN 20

// Format is used by a number of functions, so made as a file global
static struct v4l2_format fmt;

enum io_method {
    IO_METHOD_READ,
    IO_METHOD_MMAP,
    IO_METHOD_USERPTR,
};

struct buffer {
    void *start;
    size_t length;
};

static char *dev_name;
// static enum io_method   io = IO_METHOD_USERPTR;
// static enum io_method   io = IO_METHOD_READ;
static enum io_method io = IO_METHOD_MMAP;
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

static int in_crosshair(int x, int y, int cx, int cy) {
    int dx = x - cx;
    int dy = y - cy;
    return (dx == 0 && dy >= -CROSSHAIR_HALF_LEN && dy <= CROSSHAIR_HALF_LEN) ||
           (dy == 0 && dx >= -CROSSHAIR_HALF_LEN && dx <= CROSSHAIR_HALF_LEN);
}

static void process_frame(unsigned char *img, int width, int height, int frame_id) {
    struct timespec t;
    clock_gettime(CLOCK_REALTIME, &t);

    unsigned char *tmp = malloc(width * height);

    // copy borders or leave them unchanged
    for (int i = 0; i < height; i++) {
        tmp[i * width + 0] = img[i * width + 0];
        tmp[i * width + (width - 1)] = img[i * width + (width - 1)];
    }
    for (int i = 0; i < width; i++) {
        tmp[0 * width + i] = img[0 * width + i];
        tmp[(height - 1) * width + i] = img[(height - 1) * width + i];
    }

    // 3x3 Gaussian blur:
    // 1 2 1
    // 2 4 2
    // 1 2 1
    for (int i = 1; i < height - 1; i++) {
        for (int j = 1; j < width - 1; j++) {
            int sum =
                // top -> bottom, left -> right
                img[(i - 1) * width + (j - 1)] * 1 +
                img[(i - 1) * width + (j)] * 2 +
                img[(i - 1) * width + (j + 1)] * 1 +

                // center
                img[(i)*width + (j - 1)] * 2 +
                img[(i)*width + (j)] * 4 +
                img[(i)*width + (j + 1)] * 2 +

                // bottom
                img[(i + 1) * width + (j - 1)] * 1 +
                img[(i + 1) * width + (j)] * 2 +
                img[(i + 1) * width + (j + 1)] * 1;

            tmp[i * width + j] = (unsigned char)(sum / 16);
        }
    }

    // copy back
    for (int i = 0; i < width * height; i++) {
        img[i] = tmp[i];
    }

    // Sobel
    // for (int i = 1; i < height - 1; i++) {
    //     for (int j = 1; j < width - 1; j++) {
    //         // top -> bottom, left -> right
    //         int gx =
    //             img[(i - 1) * width + (j - 1)] * 1 +
    //             img[(i - 1) * width + (j)] * 0 +
    //             img[(i - 1) * width + (j + 1)] * -1 +

    //             // center
    //             img[(i)*width + (j - 1)] * 2 +
    //             img[(i)*width + (j)] * 0 +
    //             img[(i)*width + (j + 1)] * -2 +

    //             // bottom
    //             img[(i + 1) * width + (j - 1)] * 1 +
    //             img[(i + 1) * width + (j)] * 0 +
    //             img[(i + 1) * width + (j + 1)] * -1;

    //         int gy =
    //             img[(i - 1) * width + (j - 1)] * 1 +
    //             img[(i - 1) * width + (j)] * 2 +
    //             img[(i - 1) * width + (j + 1)] * 1 +

    //             // center
    //             img[(i)*width + (j - 1)] * 0 +
    //             img[(i)*width + (j)] * 0 +
    //             img[(i)*width + (j + 1)] * 0 +

    //             // bottom
    //             img[(i + 1) * width + (j - 1)] * -1 +
    //             img[(i + 1) * width + (j)] * -2 +
    //             img[(i + 1) * width + (j + 1)] * -1;

    //         int gradient = abs(gy + gx); // approx

    //         if (gradient < 255) {
    //             tmp[i * width + j] = (unsigned char)gradient;
    //         } else {
    //             tmp[i * width + j] = (unsigned char)255;
    //         }
    //     }
    // }

    // copy back
    for (int i = 0; i < width * height; i++) {
        img[i] = tmp[i];
    }

    free(tmp);

    int com_x = 0, com_y = 0;
    int row_count = 0, col_count = 0;
    // get com_x
    for (int i = 0; i < height; i++) {
        int x_first = -1, x_last = -1;
        for (int j = 0; j < width; j++) {
            int index = i * width + j;
            int pixel = img[index];
            if (pixel >= TARGET_THRESHOLD) {
                if (x_first == -1) {
                    x_first = j;
                }
                x_last = j;
            }
        }

        if (x_first != -1 && x_last != -1) {
            int mid = (x_first + x_last) / 2;
            com_x += mid;
            row_count++;
        }
    }

    com_x = row_count > 0 ? (com_x / row_count) : -1;

    for (int i = 0; i < width; i++) {
        int y_first = -1, y_last = -1;
        for (int j = 0; j < height; j++) {
            int index = j * width + i;
            int pixel = img[index];
            if (pixel >= TARGET_THRESHOLD) {
                if (y_first == -1) {
                    y_first = j;
                }
                y_last = j;
            }
        }

        if (y_first != -1 && y_last != -1) {
            int mid = (y_first + y_last) / 2;
            com_y += mid;
            col_count++;
        }
    }

    com_y = col_count > 0 ? (com_y / col_count) : -1;

    // Full whiteout method
    // for (int i = 0; i < height; i++) {
    //     for (int j = 0; j < width; j++) {
    //         int index = i * width + j;
    //         unsigned char pixel = img[index];
    //         int pixel_value = (int)pixel;
    //         if (pixel_value >= TARGET_THRESHOLD) {
    //             img[index] = 255;
    //         } else {
    //             img[index] = 0;
    //         }
    //     }
    // }

    // Mark Crosshair
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int idx = y * width + x;

            if (com_x >= 0 && com_y >= 0 && in_crosshair(x, y, com_x, com_y)) {
                img[idx] = 255;
            }
        }
    }

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
        process_input_directory("./clutter-input");
    }

    return 0;
}