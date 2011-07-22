/* Find duplicate files in current directory */
/* by Ian Brunton <iandbrunton@gmail.com> */
/* <2011-07-22 Fri> */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

typedef struct filenode {
  char file_name [256];
  size_t file_size;
  char md5 [256];
  struct filenode *next;
} filenode;

int main (int argc, char *argv[])
{
  /* get pwd */
  char pwd [256];
  getcwd (pwd, sizeof (pwd));
  printf ("pwd = %s\n", pwd);

  filenode* firstNode = NULL;
  filenode* newNode = malloc (sizeof (filenode));
  if (newNode == NULL) {
    printf ("Error allocating memory.\n");
    return;
  }

  if (firstNode == NULL) {
    firstNode = newNode;
  }

  filenode* walker = firstNode;

  /* get list of files */
  DIR *dir;
  FILE *fp;
  size_t file_size;
  struct dirent *ent;
  struct stat *fileattr = malloc (sizeof (struct stat));

  dir = opendir (pwd);
  while ((ent = readdir (dir)) != NULL) {
    if (strcmp (ent->d_name, ".\0") == 0 || strcmp (ent->d_name, "..\0") == 0)
      continue; /* don't need . and .. */

    strcpy (walker->file_name, ent->d_name);
    walker->next = malloc (sizeof (filenode));

    /* get file size */
    if (stat (walker->file_name, fileattr) == 0) {
      if (S_ISDIR (fileattr->st_mode) != 0)
	continue; /* skip directories */

      walker->file_size = fileattr->st_size;
    }
    else
      printf ("Cannot stat file %s: %s\n", walker->file_name, strerror (errno));

    walker = walker->next;
  }
  closedir (dir);

  walker = firstNode;
  while (walker->next != NULL) {
    printf ("file: %s, %d bytes\n", walker->file_name, walker->file_size);
    walker = walker->next;
  }

  /* loop through and compare file sizes */
  /* remove any non-duplicates from list */
  /* for each duplicate, compare md5 hashes */
  /* if md5s match, prompt user with both file names */
  /* give option to delete either, move to another folder, or skip */
  
  return 0;
}
