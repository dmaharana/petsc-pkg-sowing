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
 */
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

    /* Allow stdin and stdout as input, output files (so that this can be a 
       filter) */
    cmd = new CmdLine( argc, argv );

    if (!cmd->HasArg( "-debug_paths" )) (void) InstreamDebugPaths( 1 );

    /* Source and destination files */
    // ins  = new InStreamFile( "map1.C", "r" );
    ins  = new InStreamFile( );
    ins->SetBreakChar( '\n', BREAK_OTHER );
    ins->SetBreakChar( '\t', BREAK_OTHER );
    //outs = new OutStreamFile( "ccc", "w" );
    outs = new OutStreamFile( );

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
      if (!map)
	map = new TextOutMap();
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
    echo = new OutStreamFile();
    textout->SetOutstream( echo );
    while (!ins->GetToken( 256, token, &nsp )) {
	map->PutToken( nsp, token );
	//	echo->PutToken( 0, "[" );
	//echo->PutToken( 0, token );
	//echo->PutToken( 0, "]" );
	}

    delete ins;
    delete outs;
    delete map;
    delete echo;
    return 0;
}
