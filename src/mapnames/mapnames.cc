#include "cmdline.h"
#include "instream.h"
#include "outstream.h"
#include "maptok.h"
#include "textout.h"

/* 
   This is a simple program that uses the in and out streams to 
   copy one file to another, with remapping.

   Note that we MUST use pointers to an InStream/OutStream for the
   virtual functions to work.  Wierd.

   sles_user_ref

   chameleon_user_ref

   Options:
   -map mapname     - Name of map file.  For multiple map files, repeat.
                      E.g., -map map1 -map map2
   -o filename      - Set output file.  By default, stdout is used
   -printmatch      - Print all matched tokens to stdout.  Use
                      -printmatch -o /dev/null to see ONLY the matched
                      tokens.
   -inhtml          - Input is HTML.  Name mapping won't happen within HTML
                      constructions (e.g., between < and >)
   -latexout        - Output is LaTeX.  Links are writen using \href instead
                      of <A.../A>.
 */

class TextOutMapLatex : public TextOutMap {

    public:
    TextOutMapLatex( TextOut *a, int b ) :TextOutMap( a, b ) {};
/*
    TextOutMapLatex( TextOut *, int, int );
    TextOutMapLatex( );
    ~TextOutMapLatex();
*/
    virtual int PutLink( const char *, SrEntry * );
};

int main( int argc, char **argv )
{
    InStream  *ins, *mapins;
    OutStream *outs, *echo;
    TextOut   *textout = 0;
    TextOutMap *map = 0;
    CmdLine   *cmd;
    const char *mappath;
    char      token[256];
    int       nsp;
    int       pflag = 0;
    int       input_is_html = 0;

    /* Allow stdin and stdout as input, output files (so that this can be a 
       filter) */
    cmd = new CmdLine( argc, argv );

    if (!cmd->HasArg( "-debug_paths" )) (void) InstreamDebugPaths( 1 );

    if (!cmd->HasArg( "-printmatch" )) pflag = 1;

    if (!cmd->HasArg( "-inhtml" )) input_is_html = 1;

    /* Source and destination files */
    // ins  = new InStreamFile( "map1.C", "r" );
    ins  = new InStreamFile( );
    ins->SetBreakChar( '\n', BREAK_OTHER );
    ins->SetBreakChar( '\t', BREAK_OTHER );
    //outs = new OutStreamFile( "ccc", "w" );
#define MAX_FNAME 1025
    char fname[MAX_FNAME];
    if (!cmd->GetArg( "-o", fname, MAX_FNAME )) {
      outs = new OutStreamFile( fname, "w" );
      echo = new OutStreamFile( fname, "w" );
    }
    else {
      outs = new OutStreamFile( );
      echo = new OutStreamFile( );
    }

    if (!cmd->HasArg( "-html" )) {
	textout    = new TextOutHTML( outs );
	//	if (!extension) extension = "html";
	//incommands = new InStreamFile( 
	//	      DOCTEXT_PATH, "DOCTEXT_PATH", "html.def", "r" );
	}
    if (!textout) {
      // Choose simple ASCII (no interpretation)
      textout = new TextOutStrm( outs );
      textout->userops = new SrList( 127 );
      //      fprintf( stderr, "No output format specified! (-html)\n" );
      // return 1;
    }
    textout->SetDebug( 0 );

    /* Get the mapping file from the command line */
    while (!cmd->GetArgPtr( "-map", &mappath )) {
      if (!map) {
	  if (!cmd->HasArg( "-latexout" )) {
	      map = new TextOutMapLatex( 0, pflag );
	      /* printf( "Using latex output\n" ); */
	  }
	  else {
	      map = new TextOutMap( 0, pflag );
	  }
      }
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
      map->next = textout;
      textout  = map;
    }

    if (!map) {
      fprintf( stderr, "No map file specified\n" );
      return 1;
    }
    textout->SetOutstream( echo );
    while (!ins->GetToken( 256, token, &nsp )) {
        // As a special feature, we should accept a mode where the input
        // file is in HTML format, and in that case, we just skip until
        // we see the closing angle bracket
      //printf( "token [0] = %c\n", token[0] );
        if (input_is_html && token[0] == '<') {
	  //printf( "In html input\n" );
	  map->PutQuoted(0, "\0" );  // This accomplishes a flush
	  map->next->PutToken( nsp, token );
	  while (!ins->GetToken( 256, token, &nsp )) {
	    map->next->PutToken( nsp, token );
	    if (token[0] == '>') break;
	    token[0] = 0;
	  }
	  //printf( "\nLeaving html input\n" );
	  if (token[0] != '>') break;
        } 
        else {
	    map->PutToken( nsp, token );
        }
	//	echo->PutToken( 0, "[" );
	//echo->PutToken( 0, token );
	//echo->PutToken( 0, "]" );
	}

    delete ins;
    delete outs;
    delete map;
    // echo is already deleted by when the stream that it is attached to is.
    //delete echo;
    return 0;
}

/* 
   The following lets us output LaTeX instead of HTML 
 */ 

typedef struct {
    char *repname;
    char *url;
    } MapData;

int TextOutMapLatex::PutLink( const char *name, SrEntry *entry )
{
    MapData *info = (MapData *)entry->extra_data;
    next->PutToken( 0, "\\href{" );
    next->PutToken( 0, info->url );
    next->PutToken( 0, "}{" );
    next->PutToken( 0, info->repname );
    next->PutToken( 0, "}" );
    return 0;
}


/* */
