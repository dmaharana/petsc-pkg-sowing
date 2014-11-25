#include <stdio.h>
#include "sowing.h"

int main(int argc, char **argv)
{
    void *varlist, *funclist;
    SYConfigCmds cmds[3];
    int err;
    const char *var;

    SYConfigDBInit("var",&varlist);
    SYConfigDBInit("func",&funclist);
    cmds[0].name     = "var";
    cmds[0].docmd    = SYConfigDBInsert;
    cmds[0].cmdextra = varlist;
    cmds[1].name     = "func";
    cmds[1].docmd    = SYConfigDBInsert;
    cmds[1].cmdextra = funclist;

    err = SYReadConfigFile(argv[1], ' ', '#', cmds, 2);
    if (err != 1) {
	fprintf(stderr, "Error return from ReadConfig = %d\n", err );
    }
    SYConfigDBPrint(varlist,stdout);
    SYConfigDBPrint(funclist,stdout);

    /* Check for some items */
    SYConfigDBLookup("var", "errname", &var, varlist);
    if (strcmp("_ierr",var) != 0) {
	fprintf(stderr, "wrong name for errname = %s\n", var);
    }

    SYConfigDBFree(varlist);
    SYConfigDBFree(funclist);

    return 0;
}
