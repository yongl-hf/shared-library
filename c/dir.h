/*
 * @file:    dir.h
 * @author   yongl-hf@gmail.com
 */

#ifndef _DIR_H_
#define _DIR_H_

#include <sys/stat.h>
/* 
 * Description: Check whether the path is a empty directory.
 *
 * Parameters:  path - path of directory
 * Returns:     0 if it is a empty directory, or return -1. 
 */
int dir_empty(const char *path);

/*
 * Description: Remove directory resursively, including itself.
 *
 * Parameters:  path - path of directory
 * Returns: 0 if OK, -1 on error.
 */
int rmdir_r(const char *path);

/*
 * Description: attempts to create a directory named pathname, make parent directories as needed.
 *
 * Parameters:  path - path of directory
 *              mode - specifies the permissions to use
 *              uid  - user ID
 *              gid  - group ID
 * Returns: 0 if OK, -1 on error.
 */
int mkdir_p(const char *path, mode_t mode, uid_t uid, gid_t gid);



#endif
