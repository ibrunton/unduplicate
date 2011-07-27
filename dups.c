/* Find duplicate files in current directory */
/* by Ian Brunton <iandbrunton@gmail.com> */
/* <2011-07-22 Fri> */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

#define DUPE_COUNT_INIT 5

typedef struct filenode {
  char file_name [256];
  size_t file_size; /* might not need this here as well as in dupes */
  char md5 [256]; /* might not need this here as well as in dupes */
  struct filenode *next;
} filenode;

typedef struct dupenode {
  size_t size;
  char md5 [256];
  struct dupenode *next;
  short int num_dupes;
  short int num_alloc;
  filenode* duplicates [DUPE_COUNT_INIT];
} dupenode;

dupenode* dupewalker;

signed int addToDupes (filenode *fname, size_t fsize, dupenode *dupe);
signed int dupeInit (dupenode *dupe);
signed int getMD5 (dupenode *dupe, char *filename);

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
    fprintf (stderr, "Error allocating memory.\n");
    return;
  }

  if (firstFileNode == NULL) {
    firstFileNode = newFileNode;
  }

  filenode* filewalker = firstFileNode;

  /* set up linked list for duplicates */
  dupenode* firstDupeNode = NULL;
  dupenode* newDupeNode = malloc (sizeof (dupenode));
  dupeInit (newDupeNode);

  if (newDupeNode == NULL) {
    fprintf (stderr, "Error allocating memory.\n");
    return;
  }

  if (firstDupeNode == NULL) {
    firstDupeNode = newDupeNode;
  }

  /* get list of files */
  DIR *dir;
  FILE *fp;
  size_t file_size;
  int file_count = 0;
  struct dirent *ent;
  struct stat *fileattr = malloc (sizeof (struct stat));

  dir = opendir (pwd);
  while ((ent = readdir (dir)) != NULL) {
    ++file_count;
    if (stat (ent->d_name, fileattr) == 0) {
      if (S_ISDIR (fileattr->st_mode) != 0)
	continue; /* skip directories */

      strcpy (filewalker->file_name, ent->d_name);
      file_size = fileattr->st_size; /* get file size */
      filewalker->file_size = file_size; /* redundant? */

      /* if no node in dupes with file_size, add node to dupes, */
      dupewalker = firstDupeNode;
      while (dupewalker != NULL) {
	if (dupewalker->size == file_size) {
	  if (addToDupes (filewalker, file_size,
			  dupewalker) == -1) {
	    fprintf (stderr, "Error adding file %s to list of duplicates\n",
		    filewalker->file_name);
	  }
	  break;
	}
	else if (dupewalker->next == NULL) { /* last node and still not found */
	  /* add next node, with size */
	  dupewalker->next = malloc (sizeof (dupenode));
	  dupeInit (dupewalker->next);
	  dupewalker->next->size = file_size;
	  if (addToDupes (filewalker, file_size,
			  dupewalker->next) == -1) {
	    fprintf (stderr, "Error adding file %s to list of duplicates\n",
		    filewalker->file_name);
	  }
	  break;
	}
	else /* not last node, but not found yet */
	  dupewalker = dupewalker->next;
	  continue;
      }

      filewalker->next = malloc (sizeof (filenode));
    }
    else
      fprintf (stderr, "Cannot stat file %s: %s\n", filewalker->file_name,
	      strerror (errno));

    filewalker = filewalker->next;
  }
  closedir (dir);
  free (fileattr);

  dupewalker = firstDupeNode;
  int n;
  while (dupewalker->next != NULL) {
    if (dupewalker->num_dupes > 1) {
      printf ("For size %d (%d):\n", dupewalker->size, dupewalker->num_dupes);
      for (n = 0; n < dupewalker->num_dupes; n++) {
	printf ("\t%s\n", dupewalker->duplicates[n]->file_name);
      }
      printf ("\n");
    }

    dupewalker = dupewalker->next;
  }

  /* remove any non-duplicates from list */
  /* for each duplicate, compare md5 hashes */
  /* if md5s match, prompt user with both file names */
  /* give option to delete either, move to another folder, or skip */
  
  /* cleanup */
  free (newDupeNode);
  free (newFileNode);
  free (filewalker);
  free (dupewalker);
  return 0;
}

signed int addToDupes (filenode *fname, size_t fsize, dupenode *dupe)
{
  if (dupe->size != fsize)
    return -1; /* because something went screwy */

  if (dupe->num_dupes == dupe->num_alloc) { /* realloc array */
    void *_tmp = realloc (dupe, sizeof (struct dupenode) + dupe->num_alloc);
    if (_tmp == NULL) {
      fprintf (stderr, "Unable to resize duplicates struct\n");
      return -1;
    }
    dupe->num_alloc *= 2;
    dupe = _tmp;
  }
  dupe->duplicates[dupe->num_dupes++] = fname;
  return 0;
}

signed int dupeInit (dupenode *dupe)
{
  dupe->num_alloc = 5;
  dupe->num_dupes = 0;
  return 0;
}

signed int getMD5 (dupenode *dupe, char *filename)
{
  return 0;
}
