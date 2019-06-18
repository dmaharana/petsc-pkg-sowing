#ifndef RDCONFIG_H_INCLUDED
#define RDCONFIG_H_INCLUDED

typedef struct _SYConfigCmds {
    const char *name;
    int (*docmd)(const char *name, const char *key, const char *value,
		 void *extra);
    void *cmdextra;
} SYConfigCmds;

int SYReadConfigFile(const char filename[], const char sepChar,
		     const char commentChar,
		     SYConfigCmds *cmds, int ncmds);
int SYConfigDBInit(const char *cmd, void *extra);
int SYConfigDBInsert(const char *cmd, const char *key, const char *value,
		     void *extracmd);
int SYConfigDBIgnore(const char *cmd, const char *key, const char *value,
		     void *extracmd);
int SYConfigDBLookup(const char *cmd, const char *key, const char **value,
		     void *extracmd);
int SYConfigDBFree(void *extracmd);
int SYConfigDBPrint(void *extracmd, FILE *fp);

#endif /* RDCONFIG_H_INCLUDED */
