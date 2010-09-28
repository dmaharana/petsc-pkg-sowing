#include "textout.h"
#include "cmdline.h"
#include "docutil.h"
#include "docpath.h"
#include "inutil.h"
#include "maptok.h"

int DocOutRoutine( InStream *ins, TextOut *text, char *matchstring );
int DocOutMacro( InStream *ins, TextOut *text, char *matchstring );

char *IgnoreString = 0;

char NewlineString[3];
/*D
doc2lt - program to extract short descriptions in various formats

Input:
. -html     - Use tabular format output in HTML
. -mapref path - Use path as a `map` file; this contains mappings from
  names to links.  Useful mostly with '-html'.  Multiple '-mapref' files
  may be given.
. -defn path - Use 'path' to redefine the output commands.  See the default
   definition files 'latex.def' and 'html.def' for examples.
. filenames - Names the files from which documents are to be extracted

Notes:
The default format is LaTeX.

The alternate definition file 'htmltable.def' creates a Netscape-style table
for HTML. 

Author: 
William Gropp
D*/

int main( int argc, char ** argv )
{
    const char   *filename=0;
    InStream     *ins, *incommands, *mapins;
    OutStream    *outs;
    OutStreamMap *map;
    TextOut      *textout;
    CmdLine      *cmd;
    char         matchstring[5];
    const char   *path;

    cmd = new CmdLine( argc, argv );

    // Output to stdout
    outs       = new OutStreamFile();
    
    // Check for mapping files; there should be a routine to do this
    // Also, should do this ONLY for -html form (Map streams currently
    // only for HTML).
    map = 0;
    while (!cmd->GetArgPtr( "-mapref", &path )) {
    	if (!map)
	    map    = new OutStreamMap( outs, 128 );
	mapins = new InStreamFile( path, "r" );
	if (mapins->status) {
	    fprintf( stderr, "Could not read map file %s\n", path );
	    }
	else 
	    map->ReadMap( mapins );
	delete mapins;
	}
    if (map)
	outs = map;

    // Check for output format
    if (!cmd->HasArg( "-html" )) {
	textout    = new TextOutHTML( outs );
	incommands = new InStreamFile( 
		      DOCTEXT_PATH, "DOCTEXT_PATH", "html.def", "r" );
	}
    else {
	textout    = new TextOutTeX( outs );
	incommands = new InStreamFile( 
		      DOCTEXT_PATH, "DOCTEXT_PATH", "latex.def", "r" );
	}
    textout->SetDebug( 0 );
    textout->ReadCommands( incommands );
    incommands->Close();

    // Allow the user to define replacement commands
    while (!cmd->GetArgPtr( "-defn", &path )) {
	incommands = new InStreamFile( path, "r" );
	if (incommands->status) {
	    fprintf( stderr, "Could not open definition file %s\n", path );
	    }
	else 
	    textout->ReadCommands( incommands );
	incommands->Close();
	}

    textout->PutOp( "preamble" );
    while (!cmd->NextArg( &filename )) {
	ins = new InStreamFile( filename, "r" );
	if (ins->status) {
	    fprintf( stderr, "Could not open file %s\n", filename );
	    continue;
	    }
	while (!FindPattern( ins, "/*[@M]", matchstring )) {
	    // First, need to remove any sub-options (C,X)
	    DocGetSubOptions( ins );
	    if (matchstring[2] == '@') {
		/* C routine */
		DocOutRoutine( ins, textout, matchstring );
		}
	    else {
		/* Macro (M) */
		DocOutMacro( ins, textout, matchstring );
		}
	    }
	delete ins;
	}
    textout->PutOp( "postamble" );
    delete outs;
    return 0;
}

/*
   To process the structured comment for this code, we need to
   (1) Get the description
   (2) Get the synopsis

   The description is of the form 
   [space]*<name>[space]*[-]*[text,including\n]\n\n
   That is, two consequtive newlines end the description.
   
   The synopsis depends on the form.  For routines, it is read by 
   reaching the end of the comment and then reading until the first {.
   For macros, we search for a "Synopsis:" line, then read until
   two newlines.
 */

/* 
   The actual format, for LaTeX, is something like

   \k{routinename}{\CoDe{synopsis}\DeFn{definition}

   An HTML version might be

   <TR><TD>routinename</TD><TD>definition</TD></TR>

   with no synopsis.

   We can do this by using memory buffer outstreams, and then 
   calling the output routines.
 */
int DocOutRoutine( InStream *ins, TextOut *text, char *matchstring )
{
    char routinename[128];
    int  at_end;
    OutStreamBuf *outdesc, *outsynop;
    TextOutStrm  *textout;

    outdesc = new OutStreamBuf( 1024 );
    textout = new TextOutStrm( outdesc );

    if (DocReadName( ins, routinename, 128 )) return 1;
    if (DocReadDescription( ins, matchstring, textout, 0, &at_end )) return 1;
    if (!at_end) 
      if (DocSkipToFuncSynopsis( ins, matchstring )) return 1;

    outsynop = new OutStreamBuf( 16000 );
    if (DocReadFuncSynopsis( ins, outsynop )) return 1;

    // Generate the output.
    text->PutOp( "key", routinename );
    text->PutOp( "synopsis", outsynop->GetBuffer() );
    text->PutOp( "definition", outdesc->GetBuffer() );

    delete outdesc;
    delete outsynop;
    delete textout;
    
    return 0;
}

int DocOutMacro( InStream *ins, TextOut *text, char *matchstring )
{
    char routinename[128];
    OutStreamBuf *outdesc, *outsynop;
    TextOutStrm  *textout;
    int at_end;

    outdesc = new OutStreamBuf( 1024 );
    textout = new TextOutStrm( outdesc );

    if (DocReadName( ins, routinename, 128 )) return 1;
    if (DocReadDescription( ins, matchstring, textout, 0, &at_end )) return 1;
    if (at_end) {
        fprintf( stderr, "No synopsis for macro %s\n", routinename );
	return 1;
        }
    if (DocSkipToMacroSynopsis( ins, matchstring )) return 1;

    outsynop = new OutStreamBuf( 16000 );
    if (DocReadMacroSynopsis( ins, matchstring, outsynop, &at_end )) return 1;

    // Generate the output.
    text->PutOp( "key", routinename );
    text->PutOp( "synopsis", outsynop->GetBuffer() );
    text->PutOp( "definition", outdesc->GetBuffer() );
    delete outdesc;
    delete outsynop;
    delete textout;
    
    return 0;
}
