/* -*- Mode: C++; c-basic-offset:4 ; -*- */
#include "doc.h"
#include "textout.h"
#include "cmdline.h"
#include "docutil.h"
#include "dotfmt.h"
#include "inutil.h"
#include "maptok.h"
#include "keyword.h"
#include "tfile.h"
#include "docpath.h"
#include "quotefmt.h"
#include "incfiles.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

int DebugDoc = 0;
int GiveLocation = 1;
int DoQuoteFormat = 0;
char *SaveFileName = 0;
char *SaveRoutineName = 0;
// InComment is used to make sure that we don't hit EOF before finishing 
// a structured comment
int InComment = 0;

int DeTabFile = 1;
const char *IgnoreString = 0;

int DoDosFileNewlines = 0;

char NewlineString[3];

// Indicate if we're in an argument list or not
int InArgList = 0;

// Forward references
int OutputManPage( InStream *ins, TextOut *outs, char *name, char *level, 
		   const char *filename, char kind, char *date,  
		   const char *heading, const char *locdir, 
		   char *matchhstring, int one_per_file );
int OutputText( InStream *ins, char *matchstring, 
		TextOut *textout, char *name, char *level, 
	        const char *filename, char kind, char *date, 
	        const char *heading );
void MakeFileName( const char *path, char *routine, char *lextension,
	           char *outfilename );
int HandleArgs( CmdLine *cmd, const char **path, const char **extension,
                const char **incfile, const char **indexfile, 
		const char **idxdir, const char **jumpfile, 
	        const char **basedir, const char **baseoutfile, 
		const char **heading, const char **locdir );
void PrintHelp( void );

#define MAX_DATE_LEN 50
int main( int argc, char ** argv )
{
    char *outfilename;
    char *routine;
    const char *path=0;
    const char *extension;
    const char *keypath, *mappath, *defnpath, *cmdpath;
    char matchstring[20];
    char lextension[10];
    char date[MAX_DATE_LEN];
    int  masterdate;
    const char *heading=0;
    const char *incfile=0;
    const char *indexfile=0, *infilename=0, *basedir=0, *baseoutfile=0, 
      *jumpfile=0;
    FILE *idxfd, *jumpfd;
    const char *idxdir=0, *locdir;
    char kind;
    CmdLine   *cmd;
    TextOut   *textout;
    InStream  *incommands, *ins, *mapins;
    InStreamFile *insin;
    OutStream *outs;
    TextOutMap *map;
    int       one_per_file;

    cmd = new CmdLine( argc, argv );

    outfilename = new char[MAXPATHLEN];
    SaveFileName = outfilename;
    routine     = new char[MAX_ROUTINE_NAME];
    SaveRoutineName = routine;

    /* Get the many string arguments.  Set to null if not present. */
    HandleArgs( cmd, &path, &extension, &incfile, &indexfile, 
	        &idxdir, &jumpfile, &basedir, &baseoutfile, &heading, 
	        &locdir ); 
    // if (!path) path = "./";
    if (!idxdir) idxdir = ".";
    masterdate = !cmd->GetArg( "-date", date, MAX_DATE_LEN );

    if (!cmd->GetArgPtr( "-keyword", &keypath )) {
	KeywordOpen( keypath );
	}

    // I need to use exception handling to catch errors in the TextOutxxx
    // routines.
    if (!cmd->HasArg( "-html" )) {
	textout    = new TextOutHTML( );
	if (!extension) extension = "html";
	incommands = new InStreamFile( 
		      DOCTEXT_PATH, "DOCTEXT_PATH", "html.def", "r" );
	}
    else if (!cmd->HasArg( "-latex" )) {
	textout    = new TextOutTeX( );
	if (!extension) extension = "tex";
	incommands = new InStreamFile( 
		      DOCTEXT_PATH, "DOCTEXT_PATH", "latex.def", "r" );
	}
    else {
        cmd->HasArg( "-man" );    // allow -man as an option
	textout = new TextOutNroff( );
	if (!extension) extension = "3";
	incommands = new InStreamFile( 
		      DOCTEXT_PATH, "DOCTEXT_PATH", "nroff.def", "r" );
	}
    if (incommands->status) {
        fprintf( stderr, "\
Could not open base definition file for doctext-specific data\n\
You can set the environment variable DOCTEXT_PATH to the directory\n\
containing the doctext data files.  The path is .../share/doctext\n\
in the distribution, where ... is the path to the sowing directory\n\
" );
	perror( "Reason: ");
	return 1;
    }
    textout->SetDebug( 0 );
    if (DebugDoc) textout->Debug(1);
    textout->ReadCommands( incommands );
    delete incommands;

    // Allow the user to define replacement commands
    while (!cmd->GetArgPtr( "-defn", &defnpath )) {
	incommands = new InStreamFile( defnpath, "r" );
	if (incommands->status) {
	  // Try to open with the DOCTEXT_PATH (for alternative files)
	  incommands = new InStreamFile( DOCTEXT_PATH, "DOCTEXT_PATH", 
					 defnpath, "r" );
	}
	if (incommands->status) {
	    fprintf( stderr, "Could not open definition file %s\n", defnpath );
	    perror( "Reason:");
	    }
	else 
	    textout->ReadCommands( incommands );
	delete incommands;
	}

    // Allow the user to define new formatting commands.  Commands are of the
    // form (prefix)character[optional b or e].  These will invoke the commands
    // defined in the definition file.  The general actions are
    // formatting-command prefixcommand eolcommand
    // So .vb/.ve could be defined as
    // .vb s_verbatim nop
    // .ve e_verbatim nop
    // Eventually, we'll need to add the kind of processing for the text 
#ifdef FOO
    while (!cmd->GetArgPtr( "-usercmds", &cmdpath )) {
    	incommands = new InStreamFile( cmdpath, "r" );
	if (incommands->status) {
	    fprintf( stderr, "Could not open user command file %s\n", cmdpath );
	    perror( "Reason:");
	    }
	//else 
	// Read command strings
	  //textout->ReadCommands( incommands );
	incommands->Close();
	}
#endif

    // Check for mapping files; there should be a routine to do this
    // Also, should do this ONLY for -html form (Map streams currently
    // only for HTML).
    map = 0;
    while (!cmd->GetArgPtr( "-mapref", &mappath )) {
    	if (!map)
	    map    = new TextOutMap( );
	mapins = new InStreamFile( mappath, "r" );
	if (mapins->status) {
	    fprintf( stderr, "Could not read map file %s\n", mappath );
	    perror( "Reason:" );
	    }
	else 
	    map->ReadMap( mapins );
	delete mapins;
	}
    if (map) {
	if (DebugDoc) map->Debug(1);
        map->next = textout;
	textout   = map;
    }

    // Check for alternate leading string
    cmd->GetArg( "-skipprefix", LeadingString, 10 );

    // Insert quote handler if requested.
    if (!cmd->HasArg( "-quotefmt" )) 
	textout = new TextOutQFmt( textout );

    /* Open up the file of public includes */    
    SetIncludeFile( incfile );
    
    /* Open up the index file */    
    if (indexfile)
	idxfd = fopen( indexfile, "a" );
    else
	idxfd = 0;

    /* Open up the jump file */    
    if (jumpfile)
	jumpfd = fopen( jumpfile, "a" );
    else
	jumpfd = 0;

    /* Open user-selected output file */
    one_per_file = 1;
    if (baseoutfile) {
        one_per_file = 0; // All output is going to a single file
        strcpy( outfilename, baseoutfile );
	outs = new OutStreamFile( outfilename, "w" );
	if (outs->status) {
	    fprintf( stderr, "Could not open output file %s\n", outfilename );
	    perror( "Reason" );
	    return 1;
	    }
	if (DebugDoc) outs->Debug(1);
	textout->SetOutstream( outs );
	if (DoDosFileNewlines) textout->SetNewlineString( "\r\n" );
	// bof isn't correct ? Why?
	if (basedir) textout->PutOp( "bof", (char *)basedir );
	else textout->PutOp( "bof", NULL );
	// Note that bof doesn't include all that is required for
	// an html head, since in the one-file-per-page form, we
	// want to include the man page name in the <TITLE> element.
	// 
	if (textout->HasOp( "bofmanyend" ) == 0) {
	    textout->PutOp( "bofmanyend" );
	}
	//textout->PutOp( "bop" );
	}
    
    /* process all of the files */
    while (!cmd->NextArg( &infilename )) {
	if (DebugDoc) printf( "About to open %s\n", infilename );
	insin = new InStreamFile( infilename, "r" );
	if (insin->status) {
	    fprintf( stderr, "Could not open file %s\n", infilename );
	    perror( "Reason:" );
	    continue;
	    }
	if (DeTabFile) 
	    insin->SetExpandTab( 1 );

	// In case we have a large pushback
        ins = new InStreamBuf( 16000, insin );

	// We'd like to override this, particularly for testing, but also
	// for specifying a data for a release rather than the last change
	// to a file.
	if (!masterdate)
	  SYLastChangeToFile( infilename, date, (struct tm *)0 ); 
	// else already copied masterdate to date.
	  
	ClearIncludeFile( );
	/* At this point, we can trim the filename of any leading trash, 
	   such as "./".  Later.... */
	while (!FindPattern( ins, MATCH_STRING, matchstring )) {
	    // First, need to remove any sub-options (C,X)
	    InComment = 1;
	    DocGetSubOptions( ins );
	    kind = matchstring[2];
	    if (kind == INCLUDE) {
		SaveIncludeFile( ins, matchstring );
		continue;
		}
	    // Get the block name into "routine"
            DocReadName( ins, routine, MAX_ROUTINE_NAME );
	    strcpy( lextension, extension );
	    if (kind == FORTRAN)
		strcat( lextension, "f" );
	    if (kind == TEXT) {
		/* Save a block of common text */
		SaveName( ins, routine );
		continue;
		}
	    textout->SetRegisterValue( 0, routine );  // put name in putop register
	    //printf( "routine: %s\n", routine );
	    if (!baseoutfile) {
 	        one_per_file = 1;  // One manual page per "file"
		MakeFileName( path, routine, lextension, outfilename );
		outs = new OutStreamFile( outfilename, "w" );
		if (outs->status) {
		    fprintf( stderr, 
			    "Could not open output file %s\n", outfilename );
		    perror( "Reason" );
		    break;
		    }
		if (DebugDoc) outs->Debug(1);
		// This really needs to set the BOTTOM stream, incase we
		// have interposed mapref outstream handlers
		if (DoDosFileNewlines) textout->SetNewlineString( "\r\n" );
// debugging
		outs->SetBufMode(1); // debugging
		if (DebugDoc) textout->Debug(1);
// debugging
		textout->SetOutstream( outs );
		//textout->PutOp( "bop" );
		}
	    // We may also want to generate a separate index entry for each
	    // "D" object.  Why doesn't this happen?
	    if (idxfd) {
		/* If there is an indexdir, we need to remove all directories
		   from the filename */
		char *pp;
		pp = outfilename + strlen(outfilename) - 1;
		while (pp > outfilename && pp[0] != '/') pp--;
		pp++;
		fprintf( idxfd, "man:+%s++%s++++man+%s/%s#%s\n", 
			routine, routine, idxdir, pp, routine );
		}
	    if (jumpfd) {
		char rpath[1024];
		/* If there is a jumpfile, add the line */
		/* Note that we want an ABSOLUTE path for the infilename */
		SYGetRealpath( infilename, rpath );
		fprintf( jumpfd, "%s:%s:%d\n", routine, rpath, 
			 ins->GetLineNum() );
		}
	    if (!baseoutfile) {
	      // Why do we need this test?
	      if (basedir) 
		textout->PutOp( "bof", (char *)basedir );        
	      else
		textout->PutOp( "bof", NULL );
	    }
	    OutputManPage( ins, textout, routine, lextension, infilename, 
			   kind, date, heading, locdir, matchstring, 
			   one_per_file );
	    if (!baseoutfile) {
	        textout->PutOp( "eof" );
	    	textout->Flush();
		delete outs;
	        }
	    if (InComment) {
	      fprintf( stderr, "Unclosed structured comment in %s %s\n",
		       infilename, routine );
	    }
	}
	delete ins;
    }
    if (baseoutfile) {
      // This was commented out.  Why?
      textout->PutOp( "eof" );
      delete outs;
    }
    if (map) delete map;
    return 0;
}

/* 
   There are a number of things to watch for.  One is that leading blanks are
   considered significant; since the text is being formated, we usually don't
   agree with that. 
 */
int OutputManPage( InStream *ins, TextOut *textout, char *name, char *level, 
		   const char *filename, char kind, char *date, 
		   const char *heading, const char *locdir, 
		   char *matchstring, int one_per_file ) 
{
    int  at_end;

    // Output the initial information
    textout->PutOp( "bop" );
    // In the single document per page version, particularly for 
    // html, mantitle must complete the header for the file (e.g.,
    // it has some bof features).  In the multiple entries per page,
    // it must not.  We handle this by having different commands for
    // the two cases.
    if (one_per_file) {
      textout->PutOp( "mantitle", name, level, date, (char *)heading );
    }
    else {
	if (textout->HasOp( "mantitlemany" ) == 0) {
	    textout->PutOp( "mantitlemany", 
			    name, level, date, (char *)heading );
	}
	else {
	    // If no mantitlemany, just use mantitle (backwards compatibility)
	    textout->PutOp( "mantitle", name, level, date, (char *)heading );
	}
    }
    // Next, output the description
    textout->PutOp( "em_dash" );
    if (DocReadDescription( ins, matchstring, textout, 0, &at_end )) {
      if (at_end) InComment = 0;
      return 1;
    }
    if (at_end) InComment = 0;

    // Next, output the synopsis if any (kind = ROUTINE or kind = MACRO)
    // We might want a flag to indicate in-line synopsis?
    if (kind == ROUTINE || kind == PROTOTYPEDEF) {
	long position;
	ins->GetLoc( &position );
	if (!at_end)
	  DocSkipToFuncSynopsis( ins, matchstring );
	// s_synopsis should do Synopsis: <begin verbatim> and
	// e_synopsis should do <end verbatim> in most cases.
	textout->PutOp( "s_synopsis" );
        OutputIncludeInfo( textout );
	if (DocReadFuncSynopsis( ins, textout->out )) return 1;
	textout->PutOp( "e_synopsis" );
	ins->SetLoc( position );
     } 
    else if (kind == DEFINE) {
	// This is a simple way to get a define
	// definition into a doctext block. 
	long position;
	ins->GetLoc( &position );
	// Skip to func synopsis simply skips to the end of the comment
	// block
	if (!at_end)
	    DocSkipToFuncSynopsis( ins, matchstring );
	// s_synopsis should do Synopsis: <begin verbatim> and
	// e_synopsis should do <end verbatim> in most cases.
	textout->PutOp( "s_synopsis" );
        OutputIncludeInfo( textout );
	if (DocReadDefineDefinition( ins, textout->out )) return 1;
	textout->PutOp( "e_synopsis" );
	ins->SetLoc( position );
    }
    else if (kind == ENUMDEF || kind == STRUCTDEF) {
	// This is a simple way to get a struct or enum
	// definition into a doctext block.  Eventually we'll want
	// to handle enums differently by simply extracting the names
	long position;
	ins->GetLoc( &position );
	// Skip to func synopsis simply skips to the end of the comment
	// block
	if (!at_end)
	    DocSkipToFuncSynopsis( ins, matchstring );
	// s_synopsis should do Synopsis: <begin verbatim> and
	// e_synopsis should do <end verbatim> in most cases.
	textout->PutOp( "s_synopsis" );
        OutputIncludeInfo( textout );
	if (DocReadTypeDefinition( ins, textout->out )) return 1;
	textout->PutOp( "e_synopsis" );
	ins->SetLoc( position );
    }

    // Finally, output the rest of the text
    if (!at_end) 
      OutputText( ins, matchstring, 
		  textout, name, level, filename, kind, date, heading );

    /* Now add the filename where the routine or description is located */
    if (filename && GiveLocation) {
	/* Optionally, add the directory? added Dec 5, 1995. BFS */
	if (locdir) {
	    char loc[2048];
	    strcpy(loc,locdir);
	    strcat(loc,filename);
	    textout->PutOp( "location", loc );
	    }
	else {
	    textout->PutOp( "location", (char *)filename );
	    }
	}
    textout->PutOp( "eop" );
    return 0;
}

/*
    This routine generates the text part of a text page.
    It has to do several things:
    Look for the end of the text (\n<white><kind> * /<white>\n)
    Process formating directives
        a: \n<white><string>:\n
        b: \n.<string>\n
        c: \n$<string>\n
    So the rule is:
    After seeing a newline, check for . and $
    if neither, skip white space.
    Copy rest of line into buffer.
    check for <kind> * / or :\n.
    Output line accordingly.

    This needs to be split into several cases:

    May also want to add "reading arguments" to the list of special cases.

    In some source formats, all comments have a leading character string.
    This is the "LeadingString" value, it may be null.
 */        
int OutputText( InStream *ins, char *matchstring, 
		TextOut *textout, char *name, char *level, 
	        const char *filename, char kind, char *date, 
	        const char *heading )
{
    char ch;
    char lineBuffer[MAX_LINE], *lp;
    int  lastWasNl;
    int  doing_synopsis = 0;
    int  at_end, ln;
	
    lastWasNl   = 1;

    /* Note that the NAME field can't use a font changes */
    lineBuffer[0] = '+';   /* Sentinal on lineBuffer */
    at_end = 0;
    while (!at_end) {
	lp = lineBuffer + 1; ln = 1;
	if (SkipLeadingString( ins, &ch )) break;
	if (doing_synopsis && 
	    (ch == VERBATIM || ch == ARGUMENT || ch == ARGUMENT_BEGIN ||
	     ch == ARGUMENT_END || ch == '\n')) {
	    textout->PutOp( "e_synopsis" );
	    doing_synopsis = 0;
	    }
	if (ch == VERBATIM) {
	    /* Raw mode output. */
	    ProcessVerbatimFmt( ins, textout, &lastWasNl );
	    }
	else if (ch == ARGUMENT || ch == ARGUMENT_BEGIN || 
		 ch == ARGUMENT_END) {
	    ProcessDotFmt( ch, ins, textout, &lastWasNl );
	    }
	else if (ch == '\n') { 
	    if (lastWasNl) textout->PutOp( "end_par" );
	    else           textout->PutNewline();
	    lastWasNl = 1;
	    }
	/* We should allow new commands here */
	else {
	    if (isspace(ch) && ch != '\n') {
		while (!ins->GetChar( &ch ) && isspace(ch) && ch != '\n') ;
		}
	    // Handle an all blank line differently
	    if (ch == '\n') {
	      if (lastWasNl) textout->PutOp( "end_par" );
	      else           textout->PutNewline();
	      lastWasNl = 1;
	      continue;
	    }
 	    *lp++ = ch; ln++;
	    
	    /* Copy to end of line; do NOT include the EOL */
	    while (!ins->GetChar( &ch ) && ch != '\n' && ++ln < MAX_LINE) 
		*lp++ = ch;
	    if (ch != '\n') {
	      fprintf( stderr, "Input line too long\n" );
	      return 1;
	    }
	    lp--; 
	    while (isspace(*lp)) lp--;  // Note sentinal always stops us
	    lp[1] = '\0';    /* Add the trailing null */
	    // This should really look at the matchstring.
	    if (lineBuffer[1] == kind && strcmp(lineBuffer+2,"*/") == 0) {
	      InComment = 0;
	      break;
	    }
	    /* Check for escaped colon */
	    else if (lp[0] == ':' && lp[-1] == '\\') {
	        *--lp = ':';
		lp[1] = 0;
		textout->PutToken( 0, lineBuffer + 1 );
		textout->PutNewline();
	    }
	    else if (lp[0] == ':') {
		*lp = '\0';
		/* Using .SS confused the whatis builder */
		// If there is a "manual" SYNOPSIS, don't generate one later
		if (DocMatchTokens( lineBuffer+1, "SYNOPSIS" )) {
		    // Skip synopsis for routines, not MACROs!
		    // Skip not yet implmented!
		    doing_synopsis= 1;
		    textout->PutOp( "s_synopsis" );
		    if (kind == MACRO) {
		      DocReadMacroSynopsis( ins, matchstring, textout->out, 
					    &at_end );
		      doing_synopsis = 0;
		      textout->PutOp( "e_synopsis" );
		      if (at_end) InComment = 0;
		    }
		    }
		else 
		  textout->PutOp( "section", lineBuffer + 1, 2 );
		}
	    else {
		textout->PutToken( 0, lineBuffer + 1 );
		textout->PutNewline();
		}
	    }
	}
    if (doing_synopsis) 
	textout->PutOp( "e_synopsis" );
    return 0;
}

#if defined(__MSDOS__) || defined(WIN32)
#define DIR_SEP '\\'
#else
#define DIR_SEP '/'
#endif
void MakeFileName( const char *path, char *routine, char *lextension,
	           char *outfilename )
{
    outfilename[0] = 0;
    int ln;
    if (path) {
	strcat( outfilename, path );
	ln = strlen(outfilename);
	if (outfilename[ln-1] != DIR_SEP)
	    outfilename[ln] = DIR_SEP;
	    outfilename[ln+1] = 0;
        }
#if defined(__MSDOS__) || defined(WIN32)
    /* Need to use no more than 8 characters of routine (!) */
    if (strlen( routine ) > 8) {
    	char *p;
    	int  i;
    	p = outfilename + strlen(outfilename);
    	for (i=0; i<4; i++) 
    	    *p++ = routine[i];
    	routine += strlen(routine) - 4;
    	for (i=0; i<4; i++)
    	    *p++ = routine[i];
    	*p = 0;
    }
    else {
    	strcpy( outfilename, routine );
        }
#else        
    strcat( outfilename, routine );
#endif    
    strcat( outfilename, "." );
    strcat( outfilename, lextension );
}

const char *GetCurrentRoutinename( void )
{
	return SaveRoutineName ? (const char *)SaveRoutineName : "";
}

const char *GetCurrentFileName( void )
{
	return SaveFileName ? (const char *)SaveFileName : "";
}

int GetArgPtr( CmdLine *cmd, const char *name, const char **val )
{
  // use DOCTEXT_name
  char envname[128], *p;
  if (cmd->GetArgPtr( name, val )) {
    strcpy( envname, "DOCTEXT_" );
    strcat( envname, name + 1 );
    p = envname;
    while (*p) {
      *p = toupper(*p); p++ ; }
    p = getenv( envname );
    if (p) {
      char *tmp = new char[strlen(p) + 1];
      strcpy( tmp, p );
      *val = (const char *)tmp;
    }
  }
  return 0;
}

/* Routine to process the arguments.  Returns the number read */
int HandleArgs( CmdLine *cmd, const char **path, const char **extension,
                const char **incfile, const char **indexfile, 
		const char **idxdir, const char **jumpfile, 
	        const char **basedir, const char **baseoutfile, 
		const char **heading, const char **locdir )
{
  if (!cmd->HasArg( "-help" ) || !cmd->HasArg( "-h" ) || 
      !cmd->HasArg("-usage")) {
    PrintHelp( );
    exit( 0 );
  }

  if (!cmd->HasArg( "-version" ) || !cmd->HasArg( "-v" )) {
    printf( "Doctext version %s of %s\n", DOCTEXT_VERSION, DOCTEXT_DATE );
    exit( 0 );
  }
  GetArgPtr( cmd, "-mpath", path );
  GetArgPtr( cmd, "-ext", extension );
  GetArgPtr( cmd, "-I", incfile );
  GetArgPtr( cmd, "-index", indexfile );
  GetArgPtr( cmd, "-indexdir", idxdir );
  GetArgPtr( cmd, "-jumpfile", jumpfile );
  GetArgPtr( cmd, "-basedir", basedir );
  GetArgPtr( cmd, "-outfile", baseoutfile );
  GetArgPtr( cmd, "-heading", heading );
  GetArgPtr( cmd, "-locdir", locdir );
  GetArgPtr( cmd, "-ignore", &IgnoreString );

  if (!cmd->HasArg( "-debug" )) DebugDoc = 1;
  if (!cmd->HasArg( "-debug_paths" )) (void) InstreamDebugPaths( 1 );
  if (!cmd->HasArg( "-nolocation" )) GiveLocation = 0;
  if (!cmd->HasArg( "-location" )) GiveLocation = 1;
  if (!cmd->HasArg( "-dosnl" )) {
    DoDosFileNewlines = 1;
    strcpy(NewlineString, "\r\n" );
  }
  else
    strcpy( NewlineString, "\n" );

  return 0;
}

/* Print the Help page for doctext */
/*D
   doctext - Generate documentation pages from source files

Synopsis:
  doctext [ -mpath path ] [ -ext n ] [ -I filename ] [ -latex ]
          [ -html ] [ -index filename ] [ -indexdir filename ]
          [ -jumpfile filename ] [ -outfile filename ] 
          [ -mapref filename ] [ -nolocation ] [ -location ]
          [ -defn defnfile ] [ -dosnl ] [ -skipprefix name ] 
          [ -ignore string ] 
          [ -heading string ] [ -basedir dirname ] filenames

Input Parameters:
.    -mpath path -   Sets the path where the man pages will be written
.    -ext n      -   Sets the extension (1-9,l, etc)
.    -nolocation -   Don''t give the filename where the man page info was
.    -I filename -   Filename contains the public includes needed by these
                   routines
.    -latex      -   Generate latex output rather than man format files
.    -html       -   Generate html (WWW) output rather than man format files
.    -man        -   Generate man (nroff) output (default)
.    -index filename - Generate a index file appropriate for tohtml
                    (for generating WWW files)
.    -defn defnfilename - Read commands from this file after loading the
                   default commands
.    -mapref filename -  Read hyperlink database and apply to output
.    -indexdir dirname -
                   Sets the root directory for the index file entries.
                   For example
$          -index foo.cit -indexdir \"http://www.mcs.anl.gov/foo/man\"
.    -outfile filename -
                   Put the manpages in the indicated file
.    -heading name  - Name for the heading (middle of top line)
.    -quotefmt      -  support ''\\tt'' and ``\\em``
.    -skipprefix name - skip ''name'' at the beginning of each line.  This
                        may be used for Fortran or Shell sources
.    -ignore string - skip ''name'' in a function synopsis.  This can be used 
                      to remove special keywords needed to build the routine
                      but not needed by the user (e.g., special export
                      keywords when building DLLs)   
.    -keyword filename -
                   Place keyword entries at the end of the specified file
.    -locdir directory -
                   Uses directory in the Location: field
.    -dosnl - Generate DOS style files (return-newline instead of newline)
.    filenames -      Names of the files from which documents are to be
                   extracted
D*/
void PrintHelp( void )
{
fprintf( stderr, "\
doctext [ -mpath path ] [ -ext n ] [ -I filename ] [ -latex ]\n\
        [ -html ] [ -index filename ] [ -indexdir filename ]\n\
        [ -jumpfile filename ] [ -outfile filename ] \n\
        [ -mapref filename ] [ -nolocation ] [ -location ]\n\
        [ -defn defnfile ] [ -dosnl ] [ -skipprefix name ]\n\
        [ -debug_paths ]\n\
        [ -heading string ] [ -basedir dirname ] filenames\n\
\n\
    -mpath path    Sets the path where the man pages will be written\n\
    -ext n         Sets the extension (1-9,l, etc)\n\
    -nolocation    Don't give the filename where the man page info was\n\
    -I filename    Filename contains the public includes needed by these\n\
                   routines\n\
    -latex         Generate latex output rather than man format files\n\
    -html          Generate html (WWW) output rather than man format files\n\
    -index filename Generate a index file appropriate for tohtml\n\
                    (for generating WWW files)\n" );
fprintf( stderr, "\
    -defn defnfilename Read commands from this file after loading the \n\
                   default commands.\n\
    -mapref filename   Read hyperlink database and apply to output\n" );
fprintf( stderr, "\
    -indexdir dirname \n\
                   Sets the root directory for the index file entries.\n\
                   For example\n\
          -index foo.cit -indexdir \"http://www.mcs.anl.gov/foo/man\"\n\
    -outfile filename\n\
                   Put the manpages in the indicated file. \n\
    -heading name  Name for the heading (middle of top line)\n\
    -quotefmt      support '\\tt' and `\\em`\n\
    -skipprefix name - skip ''name'' at the beginning of each line.  This\n\
                        may be used for Fortran or Shell sources\n\
    -keyword filename\n\
                   Place keyword entries at the end of the specified file\n" );
fprintf( stderr, "\
    -locdir directory\n\
                   Uses directory in the Location: field\n\
    -dosnl         Generate DOS style files (return-newline instead of newline)\n\
    -debug_paths   Provide information about the paths being searched\n\
                   for various files, including the definition files used\n\
                   to define the output formats.  Use this if doctext \n\
                   complains that it cannot find files like html.def or\n\
                   nroff.def\n\
    filenames      Names of the files from which documents are to be\n\
                   extracted\n\
\n\
See the manual for details\n" );
}

