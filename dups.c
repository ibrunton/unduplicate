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

typedef struct dupenode {
  size_t size;
  filenode *duplicates;
  struct dupenode *next;
} dupenode;

int main (int argc, char *argv[])
{
  /* get pwd */
  char pwd [256];
  getcwd (pwd, sizeof (pwd));
  printf ("pwd = %s\n", pwd);

  /* set up linked list of files */
  filenode* firstFileNode = NULL;
  filenode* newFileNode = malloc (sizeof (filenode));
  if (newFileNode == NULL) {
    printf ("Error allocating memory.\n");
    return;
  }

  if (firstFileNode == NULL) {
    firstFileNode = newFileNode;
  }

  filenode* filewalker = firstFileNode;

  /* set up linked list for duplicates */
  dupenode* firstDupeNode = NULL;
  dupenode* newDupeNode = malloc (sizeof (dupenode));
  if (newDupeNode == NULL) {
    printf ("Error allocating memory.\n");
    return;
  }

  if (firstDupeNode == NULL) {
    firstDupeNode = newDupeNode;
  }

  dupenode* dupewalker;

  /* get list of files */
  DIR *dir;
  FILE *fp;
  size_t file_size;
  struct dirent *ent;
  struct stat *fileattr = malloc (sizeof (struct stat));

  dir = opendir (pwd);
  while ((ent = readdir (dir)) != NULL) {
    if (stat (ent->d_name, fileattr) == 0) {
      if (S_ISDIR (fileattr->st_mode) != 0)
	continue; /* skip directories */

      strcpy (filewalker->file_name, ent->d_name);
      filewalker->file_size = fileattr->st_size; /* get file size */

      /* if no node in dupes with file_size, add node to dupes, */
      dupewalker = firstDupeNode;
      while (dupewalker != NULL) {
	if (dupewalker->size == filewalker->file_size) {
	  /* ... */

	  break;
	}
	else if (dupewalker->next == NULL) { /* last node and still not found */
	  /* add next node, with size */
	  dupewalker->next = malloc (sizeof (dupenode));
	  dupewalker->next->size = filewalker->file_size;
	  /* add pointer to filewalker... */
	  break;
	}
	else /* not last node, but not found yet */
	  continue;
      }

      filewalker->next = malloc (sizeof (filenode));
    }
    else
      printf ("Cannot stat file %s: %s\n", filewalker->file_name, strerror (errno));

    filewalker = filewalker->next;
  }
  closedir (dir);

  filewalker = firstFileNode;
  while (filewalker->next != NULL) {
    printf ("file: %s, %d bytes\n", filewalker->file_name, filewalker->file_size);
    filewalker = filewalker->next;
  }

  /* loop through and compare file sizes */
  filewalker = firstFileNode;
  filenode* filewalker2;
  while (filewalker->next != NULL) {
    filewalker2 = firstFileNode;
    printf ("%s has duplicates: ", filewalker->file_name);
    while (filewalker2->next != NULL) {
      if (strcmp (filewalker2->file_name, filewalker->file_name) != 0) {
	if (filewalker->file_size == filewalker2->file_size) {
	  printf ("%s ", filewalker2->file_name);
	}
      }
      filewalker2 = filewalker2->next;
    }
    printf ("\n");
    filewalker = filewalker->next;
  }
  /* remove any non-duplicates from list */
  /* for each duplicate, compare md5 hashes */
  /* if md5s match, prompt user with both file names */
  /* give option to delete either, move to another folder, or skip */
  
  return 0;
}
