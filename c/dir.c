/*
 * @file:    dir.c
 * @author   yongl.hf@gmail.com
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>

#ifndef NAME_MAX
#define NAME_MAX 255
#endif
#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

int dir_empty(const char *path)
{
    int     ret = 0;
    DIR    *dp = NULL;
    struct dirent *d = NULL;
    struct dirent *result = NULL;

    if ((dp = opendir(path)) == NULL) {
        return -1;
    }

    d = (struct dirent *) malloc(sizeof(struct dirent) + NAME_MAX);
    if (d == NULL) {
        ret = -1;
        goto out;
    }

    while (1) {
        ret = readdir_r(dp, d, &result);
        if (ret != 0 || result == NULL) {
            break;
        }
        if (strcmp(d->d_name, ".") != 0 && strcmp(d->d_name, "..") != 0) {
            ret = -1;
            break;
        }
    }

out:
    if (dp) {
        closedir(dp);
    }
    if (d) {
        free(d);
    }
    return ret;
}

/* 
 * Internal process， call by rmdir_r().
 * remove directory resursively, expect itself.
 *
 * Returns: 0 if OK, -1 on error.
 */
static int _rmdir_r(char *path)
{
    int     ret = 0;
    int     len = 0;
    DIR    *dp = NULL;
    struct dirent *d = NULL;
    struct dirent *result = NULL;
    struct stat st;

    len = strlen(path);
    if ((dp = opendir(path)) == NULL) {
        return -1;
    }

    d = (struct dirent *) malloc(sizeof(struct dirent) + NAME_MAX);
    if (d == NULL) {
        ret = -1;
        goto out;
    }

    while (1) {
        ret = readdir_r(dp, d, &result);
        if (ret != 0 || result == NULL) {
            break;
        }
        if (strcmp(d->d_name, ".") == 0 || strcmp(d->d_name, "..") == 0) {
            continue;
        }

        snprintf(path + len, strlen(d->d_name) + 2, "/%s", d->d_name);
        if (lstat(path, &st) != 0) {
            ret = -1;
            continue;
        }

        if (S_ISDIR(st.st_mode)) {
            _rmdir_r(path);
            snprintf(path + len, strlen(d->d_name) + 2, "/%s", d->d_name);
            ret = rmdir(path);
            if (ret < 0 && errno != ENOENT) {
                ret = -1;
                continue;
            }
        } else {
            if (unlink(path) != 0) {
                ret = -1;
                continue;
            }
        }
    }
out:
    if (dp) {
        closedir(dp);
    }
    if (d) {
        free(d);
    }
    return ret;
}

int rmdir_r(const char *path)
{
    int     ret = 0;
    char    buf[PATH_MAX];

    memset(buf, 0, PATH_MAX);
    snprintf(buf, sizeof(buf), "%s", path);

    ret = _rmdir_r(buf);
    if (ret < 0) {
        return -1;
    }
    ret = rmdir(path);
    if (ret < 0 && errno != ENOENT) {
        return -1;
    }
    return 0;
}

int mkdir_p(const char *path, mode_t mode, uid_t uid, gid_t gid)
{
    int     len = 0;
    int     ret = 0;
    char    buf[PATH_MAX];
    char   *pos = NULL;
    char   *pwd = NULL;
    struct stat st;

    memset(buf, 0, PATH_MAX);
    if (path == NULL) {
        return -1;
    }
    /* Relatively directory. */
    if (path[0] != '/') {
        pwd = getcwd(buf, PATH_MAX);
        if (pwd == NULL) {
            return -1;
        }
    }
    len = (pwd == NULL) ? 0 : strlen(pwd);
    snprintf(buf + len, PATH_MAX - len, "/%s", path);
    pos = buf + len;

    len = strlen(buf);
    if (buf[len - 1] != '/') {
        buf[len] = '/';
    }

    if (lstat(buf, &st) == 0) {
        return 0;
    }

    for (;;) {
        pos++;
        pos = strchr(pos, '/');
        if (pos == NULL) {
            break;
        }
        *pos = '\0';
        if (lstat(buf, &st) == 0) {
            if (S_ISDIR(st.st_mode)) {
                *pos = '/';
                continue;
            } else {
                return -1;
            }
        }

        ret = mkdir(buf, mode);
        if (ret == -1 && errno != EEXIST) {
            return -1;
        } else {
            lchown(buf, uid, gid);
            *pos = '/';
        }
    }
    return 0;
}
