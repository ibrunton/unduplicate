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

char *programName;
char directory [256];
int prompt;

signed int addToDupes (filenode *fname, size_t fsize, dupenode *dupe);
signed int dupeInit (dupenode *dupe);
signed int dupeMenu ();
signed int getMD5 (dupenode *dupe, char *filename);
void printUsage ();

int main (int argc, char *argv[])
{
  programName = malloc (sizeof (argv[0]));
  strcpy (programName, argv[0]);

  getcwd (directory, sizeof (directory));

  /* get options */
  int i;
  for (i = 1; i < argc; i++) {
    if (argv[i][0] == '-') {
      switch (argv[i][1]) {
      case 'p':
	prompt = 1;
	break;
      default:
	fprintf (stderr, "Unknown option: %s\n", argv[i]);
	return 1;
      }
    }
    else { /* is directory argument */
      strcpy (directory, argv[i]);
    }
  }

  printf ("directory = %s\n", directory);

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

  dir = opendir (directory);
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

  /* output list of duplicate files */
  dupewalker = firstDupeNode;
  int n;
  signed int dupe_action;
  while (dupewalker->next != NULL) {
    if (dupewalker->num_dupes > 1) {
      printf ("For size %d (%d):\n", dupewalker->size, dupewalker->num_dupes);
      for (n = 0; n < dupewalker->num_dupes; n++) {
	printf ("%d:\t%s\n", n + 1, dupewalker->duplicates[n]->file_name);
      }
      if (prompt == 1) {
	dupe_action = dupeMenu ();
	if (dupe_action > 0) { /* delete */
	  printf ("Deleting file %s...\n",
		  dupewalker->duplicates[dupe_action - 1]->file_name);
	  /* remove (dupewalker->duplicates[dupe_action -1]->file_name); */
	}
	else if (dupe_action < 0) { /* move */
	  printf ("Moving file %s...\n",
		  dupewalker->duplicates[(dupe_action + ((0 - dupe_action) * 2)) - 1]->file_name);
	}
	else {
	  printf ("Skipping.\n");
	}
      }

      printf ("\n");
    }

    dupewalker = dupewalker->next;
  }

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

signed int dupeMenu ()
{
  int menuin, action_file;
  char selection, action;

  printf ("[d] delete [m] move [s] skip\n");
  printf ("Enter action (d/m) and file index, or s to skip: ");

  while ((menuin = getchar ()) != '\n') {
    selection = (char)menuin;
    if (selection == 'd' || selection == 'm' || selection == 's')
      action = selection;
    else if (menuin > 47 && menuin < 58)
      action_file = menuin - 48;
    else
      continue;
  }

  if (action == 'd') {
    return action_file;
  }
  else if (action == 'm') {
    return 0 - action_file;
  }
  else if (action == 's')
    return 0;
}

signed int getMD5 (dupenode *dupe, char *filename)
{
  return 0;
}

void printUsage ()
{
  fprintf (stderr, "usage: %s [-p] [directory]\n", programName);
  return;
}
