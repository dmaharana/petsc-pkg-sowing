/* -*- Mode: C; c-basic-offset:4 ; -*- */
/*
 * Read a configuration file.
 * The format is simple but relatively general:
 *
 * [command<sep>]key[<sep>value]
 *
 * Commands:
 *  Predefined:
 *    include - key is file name; this provides an indirect to another file.
 *    #       - comment.  Rest of line is discarded (specific character
 *              is a parameter - see commentChar)
 *  User defined:
 *    cmd - key and optional value are read and passed to the function
 *          associated with cmd.
 * <sep> is the separator character (parameter sepChar).
 *
 */
#include <stdio.h>
#include <ctype.h>
#include "sowing.h"

#define BUFSIZE 1024
#define escChar '\\'

int SYReadConfigFile(const char filename[], const char sepChar,
		     const char commentChar,
		     SYConfigCmds *cmds, int ncmds)
{
    FILE *fp;
    char *buf, *bufp, *cmd, *key, *value;
    int  i, err;

    fp = fopen(filename, "r");
    if (!fp) {
	fprintf(stderr, "Unable to open config file %s\n", filename);
	return 0;
    }
    buf = MALLOC(BUFSIZE);  CHKPTRN(buf);
    while (fgets(buf, BUFSIZE-1, fp)) {
	buf[BUFSIZE-1] = 0; /* Just in case */
	bufp = buf; cmd = NULL; key = NULL; value = NULL;
	while (*bufp && isspace(*bufp) && *bufp != '\n') bufp++;
	if (*bufp == commentChar) continue;
	/* Extract items */
	cmd = bufp++;
	while (*bufp && *bufp != sepChar && *bufp != '\n') {
	    if (*bufp == escChar && bufp[1]) bufp++;
	    bufp++;
	}
	/* Terminate the cmd */
	if (*bufp) {
	    *bufp++ = 0;
	}
	/* Skip empty commands */
	if (*cmd == 0 || *cmd == '\n') continue;
	/* Look for the key */
	while (*bufp && isspace(*bufp) && *bufp != '\n') bufp++;
	key = bufp;
	while (*bufp && *bufp != sepChar && *bufp != '\n') {
	    if (*bufp == escChar && bufp[1]) bufp++;
	    bufp++;
	}
	/* Terminate the key */
	if (*bufp) {
	    *bufp++ = 0;
	}
	while (*bufp && isspace(*bufp) && *bufp != '\n') bufp++;
	value = bufp;
	while (*bufp && *bufp != sepChar && *bufp != '\n') {
	    if (*bufp == escChar && bufp[1]) bufp++;
	    bufp++;
	}
	/* Terminate the value */
	if (*bufp) {
	    *bufp++ = 0;
	}
	/* check for predefined commands */
	if (strcmp(cmd,"include") == 0) {
	    if (key == NULL || *key == 0) {
		fprintf(stderr,"include command with no file\n");
		return 0;
	    }
	    if (value != NULL && *value != 0) {
		fprintf(stderr,"include command improperly formed: %s\n",
			buf);
		return 0;
	    }
	    /* Look for file first.  If not found, look in the same
	       directory as the current file, from the filename */
	    if (SYiFileExists(key,'r')) {
		err = SYReadConfigFile(key, sepChar, commentChar, cmds, ncmds);
	    }
	    else if (*key != '/') {
		char *fname=0, *pout;
		const char *pin = filename;
		fname = MALLOC(strlen(filename)+strlen(key)+2);
		CHKPTRN(fname);
		pout = fname;
		while (*pin) {
		    *pout++ = *pin++;
		}
		*pout = 0;
		/* Backup to the very first / */
		while (pout > fname && *pout != '/') pout--;
		if (pout > fname) {
		    pout++;  /* Position after the last / */
		    /* Append the file name */
		    pin = key;
		    while (*pin) {
			*pout++ = *pin++;
		    }
		    *pout = 0;
		    if (SYiFileExists(fname,'r')) {
			err = SYReadConfigFile(fname, sepChar, commentChar,
					       cmds, ncmds);
		    }
		    else {
			fprintf(stderr,"include config file %s missing\n",
				fname );
			return 0;
		    }
		}
		else {
		    fprintf(stderr,"include command improperly formed: %s\n",
			    buf);
		    return 0;
		}
		FREE(fname);
	    }
	    if (err != 1) {
		fprintf(stderr,"Error when reading include file\n");
		return err;
	    }
	}
	else {
	    /* Find cmd in the list. Because the number of commands
	       is supposed to be relatively small, the linear search
	       is best */
	    int found=0;
	    for (i=0; i<ncmds; i++) {
		if (cmds[i].name == NULL) break;
		if (strcmp(cmd,cmds[i].name) == 0) {
		    err = (cmds[i].docmd)(cmd, key, value, cmds[i].cmdextra);
		    found = 1;
		    break;
		}
	    }
	    if (!found) {
		fprintf(stderr, "%s: Command %s unknown\n",
			filename, cmd);
	    }
	}
    }

    fclose(fp);
    FREE(buf);
    return 1;
}

/* Provide routines to support simple insert and lookup of data items */
/* There are separate lists for each command, with the data saved in
   the extracmd. */

typedef struct _sydbnode {
    const char *key;
    const char *value;
    struct _sylookupnode *next;
} SYDBNode;

typedef struct _sydb {
    int         nalloc;
    int         nused;
    const char *name;
    SYDBNode   *items;
} SYDB;

int SYConfigDBInit(const char *cmd, void *extra)
{
    SYDB *db;
    db         = (SYDB *)MALLOC(sizeof(SYDB));  CHKPTRN(db);
    db->items  = (SYDBNode *)MALLOC(64*sizeof(SYDBNode)); CHKPTRN(db->items);
    db->nused  = 0;
    db->nalloc = 64;
    db->name   = STRDUP(cmd);  CHKPTRN(db->name);
    *(void **)extra = (void *)db;
    return 1;
}

int SYConfigDBInsert(const char *cmd, const char *key, const char *value,
		     void *extracmd)
{
    SYDB     *db = (SYDB *)extracmd;
    SYDBNode *node = 0;
    int       i;

    if (strcmp(db->name,cmd) != 0) {
	/* inconsistent data structures */
	return 0;
    }
    for (i=0; i<db->nused; i++) {
	if (strcmp(db->items[i].key, key) == 0) {
	    /* Duplicate entry */
	    if (db->items[i].value) {
		FREE( db->items[i].value );
	    }
	    db->items[i].value = value ? STRDUP(value) : 0;
	    return 0;
	}
    }
    if (db->nused == db->nalloc) {
	/* Need to extend the list */
	SYDBNode *newnodes = (SYDBNode *)MALLOC(db->nalloc*2*sizeof(SYDBNode));
	CHKPTRN(newnodes);
	for (i=0; i<db->nalloc; i++) {
	    newnodes[i] = db->items[i];
	}
	FREE(db->items);
	db->items = newnodes;
	db->nalloc *= 2;
    }
    node = &db->items[db->nused++];
    node->key   = STRDUP(key);
    node->value = value ? STRDUP(value) : 0;

    return 1;
}

/*
 Use this as the routine to call for commands to be ignored.
 */
int SYConfigDBIgnore(const char *cmd, const char *key, const char *value,
		     void *extracmd)
{
    return 1;
}

/*
Output Parmeters:
. value - If non-null, the value associated with the key

Return Value:
1 on success (value found), 0 on not found, and -1 on error.
 */
int SYConfigDBLookup(const char *cmd, const char *key, const char **value,
		     void *extracmd)
{
    SYDB     *db = (SYDB *)extracmd;
    int       i;

    if (strcmp(db->name,cmd) != 0) {
	/* inconsistent data structures */
	return -1;
    }

    for (i=0; i<db->nused; i++) {
	if (strcmp(db->items[i].key, key) == 0) {
	    if (value)
		*value = db->items[i].value;
	    return 1;
	}
    }
    /* Not found */
    if (value)
	*value = (const char *)0;
    return 0;
}

int SYConfigDBFree(void *extracmd)
{
    SYDB *db = (SYDB *)extracmd;

    FREE(db->items);
    FREE(db);
    return 1;
}

int SYConfigDBPrint(void *extracmd, FILE *fp)
{
    SYDB *db = (SYDB *)extracmd;
    int  i;

    for (i=0; i<db->nused; i++) {
	fprintf(fp,"%s:%s\n", db->items[i].key,
		db->items[i].value ? db->items[i].value : "<null>" );
    }
    return 1;
}
