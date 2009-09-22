#ifndef _TEX
#define _TEX

#include "sowingconfig.h"

#ifdef STDC_HEADERS
#include <stdlib.h>
#include <string.h>
#endif

/* Paths for the various source files */
#include "tohtmlpath.h"

typedef struct _TeXEntry {
    void (*action)( struct _TeXEntry * );     
                          /* Action to perform for entry */
    void  *ctx;           /* Context to pass to action */
    int  nargs;           /* Number of arguments to TeX command */
    char *name;           /* pointer to name */
    } TeXEntry;

/* The latex stack */
typedef enum { TXITEMIZE, TXDESCRIPTION, TXEXAMPLE, TXVERBATIM, TXENUMERATE,
	       TXLIST, TXTABULAR } 
        EnvType;
typedef struct {
    EnvType env;
    int    num;            /* relative number of an item in this
			      environment (needed for enumerate) */
    void    (*newline)( FILE *);   
                           /* routine to call for new-line handling */
    char   *p1, *p2;       /* Pointers to text for use by the environment
                              (some have code and replacement text; 
			      principly a user-defined list environment */
    /* These are currently unused */
    char   *label_node_name; /* Name of the node */
    char   *label_text;      /* Text to use in refering to the label */
    /* This is used to keep track of where an environment started */
    int    line_num;
    /* Special data for processing an environment */
    void   *extra_data;
    } LaTeXStack;

#define MAX_TEX_STACK 20    
extern LaTeXStack lstack[MAX_TEX_STACK];
extern int    lSp;

extern int    curfile;
extern FILE *(fpin[10]);  /* Input file stack */
extern FILE *fpout;       /* Output file */
extern char *outfile;     /* Name of output file */
extern FILE *ferr;        /* Error report file */

extern int  splitlevel;
extern char splitdir[100];/* directory for output files */

extern char userpath[1024];  /* Path to search for package definitions */

extern char ImageExt[4];     /* Choose the image type.  Used as both the
			        image name and the file extension.  
			        Currently either xbm or gif */

extern char latex_errname[300];  /* File name used for errors in 
				    generating latex or graphics files */

extern int InDocument;

extern int InOutputBody;

extern int LineNo[10];    /* Line number in current file (LineNo[curfile]) */
extern char *(InFName[10]); /* Names of files currently open */
extern int BraceCount;
extern char CmdName[65];   /* Used to hold a command name for error reporting */

extern int InArg;         /* Set to 1 if in TXGenGetArg; tells the 
			     output routines to write into ArgBuffer
			     instead */
extern char *ArgBuffer;

extern int UsingLatexinfo;    /* Latexinfo has a number of special features */

extern int DestIsHtml;    /* Destination language is html */
extern int HTMLv3;        /* HTML version 3(.2) rather than earlier */

extern char *preamble;    /* documentstyle, etc */
extern char *predoc;      /* Important TeX commands encountered before the
			     \begin{document} */
extern char *documentcmd;
extern int DoGaudy;       /* If true, generate more colorful output */

/* These control the format of a citation name.  We should probably use
   a definition for @cite, but this is easier to toss in... 
 */
extern char *CitePrefix;
extern char *CiteSuffix;

/* These control whether we generate GIF files for what we don't understand */
extern int LatexUnknownEnvs;
/* Latex Tables? */
extern int LatexTables;
/* Latex Math? */
extern int LatexMath;
/* Simple Math (math mode with no TeX commands done in italics */
extern int SimpleMath;
/* Number of the generated image file */
extern int imageno;     
/* Set LatexAgain to 0 if you want to use the old img files */
extern int LatexAgain;

/* Set to 1 if using HTML table commands */
extern int HandleAlign;

/* Use these to change to DOS-style names */
extern char HTML_Suffix[5];
extern char DirSep;
extern char DirSepString[2];
extern int  DoDosFileNewlines;
extern char NewLineString[4];

/* Table, Figure, and Equation numbering.

   Note that what we really need is for each environment TYPE to have
   a counter, so that user-defined environment will work properly.  
   What we have here works for the simple cases 
 */
extern int TableNumber, FigureNumber, EquationNumber;

/* The kind of the environment.  This assumes that they are NOT nested;
   this is ok for tables, equations, and figures (well, not quite, as
   a table or figure might have an equation) 
 */
#define ENV_NONE     0
#define ENV_TABLE    1
#define ENV_FIGURE   2
#define ENV_EQUATION 3
extern int NumberedEnvironmentType;

extern char *envjumpname;
extern int  envJumpNum;

extern int NoBMCopy;

extern int  DoOutputTranslation;
extern int  DoActiveTokens;

extern int IncludeSectionNumbers;
#include "search.h"
extern SRList *TeXlist;

/* Index names */
extern char *UserIndexName;

/* Catcode processing */
extern int IgnoreCatcode;

/*
   We might as well do more native TeX processing, by keeping track of
   TeX's special character just as TeX does.  These are
 */
extern char CommandChar;     /* often \ */
extern char SubscriptChar;   /* often _ */
extern char SuperscriptChar; /* often ^ */
extern char MathmodeChar;    /* often $ */
extern char CommentChar;     /* often % */
extern char LbraceChar;      /* often { */
extern char RbraceChar;      /* often } */
extern char ArgChar;         /* often # */
extern char AlignChar;       /* often & */
extern char ActiveChar;      /* often ~ */

/* General globals */
extern int  DebugCommands;
extern int  DebugOutput;
extern int  DebugFont;
extern int  warnRedefinition;

extern char *tokbuf, *curtok;
extern int  toknum;
#define MAX_CURTOKS 15
#define PUSHCURTOK {curtok += MAX_TOKEN; toknum++;\
    if (toknum >= MAX_CURTOKS) TeXAbort( "PUSHCURTOK", "Tokens too deep"); }
#define POPCURTOK  {curtok -= MAX_TOKEN; toknum--; \
    if (toknum < 0) TeXAbort("POPCURTOK", "Tokens popped too high");}

/* Token management */
/* In order to protect 'output' tokens from re-evaluation, they are protected 
   with non-printing ASCII characters. */
/* #define TOK_TEST */
#ifdef TOK_TEST
#define TOK_START ((unsigned char)'^')
#define TOK_END   ((unsigned char)'#')
#else
#define TOK_START ((unsigned char)1)
#define TOK_END   ((unsigned char)2)
#endif

/* Functions */

/* accent.c */
void InitAccents( void );

/* biblio.c */
extern void TXbibitem ( TeXEntry * );
extern void InsertBib ( int, char * );
extern int BibLookup ( char *, char **, char ** );
extern void TXDoBibliography ( TeXEntry * );

/* dimen.c */
extern int TXReadDimen ( FILE * );
extern void TXdimen ( TeXEntry * );
extern void TXnumber ( TeXEntry * );
extern void TXcounter ( TeXEntry * );
extern void TXadvance ( TeXEntry * );

/* environ.c */
extern void TXSetLatexQuiet ( int );
extern void TXDoNewenvironment ( TeXEntry * );
extern void TXDoNewtheorem ( TeXEntry * );
extern int LookupEnv ( char *, char **, char **, int * );
extern void PushBeginEnv ( char *, int );
extern void RunLatex ( char *, char *, char *, char *, char *, int );
extern void TXStartNumberedEnv ( char * );
extern void TXcaptionHandling ( TeXEntry * );
extern void TXcaption ( TeXEntry * );
extern void TeXSetEnvJump ( char * );
extern void TXSetEnv( const char *, char *, char *, int );

/* label.c */
extern void InsertLabel ( int, char *, char * );
extern char *LabelLookup ( char *, int *, char * );
extern void ReplaceWhite ( char * );

/* math.c */
extern void TeXskipMath ( TeXEntry *, char *, int );
extern void TXProcessDollar ( TeXEntry *, int, int );

/* rdaux.c */
extern void OpenAuxFile ( char * );
extern void OpenWRAuxFile ( void );
extern void RewindAuxFile ( void );
extern void RdAuxFile ( SRList * );
extern void WRtoauxfile ( int, char *, int, char * );
extern void WriteLabeltoauxfile ( int, char *, char *, char * );
extern void WriteBibtoauxfile ( int, char *, char * );
extern void WRfromauxfile ( FILE *, int );
extern int NumChildren ( LINK * );
extern void WriteChildren ( FILE *, LINK *, int );
extern int GetParent ( LINK *, char *, char *, char * );
extern int GetNext ( LINK *, char *, char *, char * );
extern LINK *GetNextLink ( LINK * );
extern int GetPrevious ( LINK *, char *, char *, char * );
extern char *TopicFilename ( LINK * );

/* rddefs.c */
extern void RdBaseDef ( char * );
extern char *TXConvertQuotes( char *, char  );
extern void TXInitialCommands( void );

/* rdindx.c */
extern void AddToIndex ( char *, char *, int, int );
extern void WriteIndex ( FILE *, int );

/* refmap */
extern void RdRefMap ( char * );
extern int RefMapLookup ( char *, char *, char **, int *, char ** );

/* scan.c */
extern int (*SCSetTranslate ( int (*)(char *,int) )) 
              ( char *, int );
extern void SCSetDebug ( int );
extern void SCSetAtLetter ( int );
extern void SCInitChartype ( void );
extern int SCTxtFindNextANToken ( FILE *, char *, int, int * );
extern int SCTxtGetChar ( FILE * );
extern void SCAppendToken ( char * );
extern void SCPushToken ( char * );
extern void SCPushCommand ( char * );
extern void SCPushChar ( char );
extern void SCSetCommentChar ( char );
extern void SCSkipNewlines ( FILE * );
extern char SCGetCommentChar ( void );

/* simpleif.c */
extern void TXNewif( TeXEntry * );
extern void TXElse( TeXEntry * );
extern void TXFi( TeXEntry * );
extern void TXAddPredefIf( const char *, int );

/* style.c */
extern void TXStyleLatexInfo ( SRList *, FILE *, FILE * );
extern void TXStyleFunclist ( SRList *, FILE *, FILE * );
extern void TXANLTitle ( TeXEntry * );
extern void TXStyleTpage ( SRList *, FILE *, FILE * );
extern void TXStyleTools ( SRList *, FILE *, FILE * );
extern void TXStyleEPSF ( SRList *, FILE *, FILE * );
extern void TXStyleAnlhtext ( SRList *, FILE *, FILE * );
extern void TXmakeinfo ( TeXEntry * );
extern void TXStyleANLHandpage ( SRList *, FILE *, FILE * );
extern void TXStylePsfig ( SRList *, FILE *, FILE * );

/* tabular.c */
extern void TeXtabular ( TeXEntry * );
extern void TeXGetTabularDefn( void );
extern void TeXBeginHalignTable( void );
extern void TeXEndHalignTable( void );

/* texactio.c */
extern void TXSetDebug ( int );
extern void TXSetDebugFile( int );
extern void TXSetDebugFont( int );
extern void TXSetUseIfTex ( int );
extern void TXSetLatexUnknown ( int );
extern void TXSetProcessManPageTokens ( int );
extern void TXSetLatexTables ( int );
extern void TXSetLatexMath ( int );
extern void TXSetSimpleMath ( int );
extern void TXSetLatexAgain ( int );
extern void TXSetFiles ( char *, char * );
extern void TeXAbort ( char *, char * );
extern void TXPrintLocation( FILE * );
extern void TeXoutcmd ( FILE *, char * );
extern void TeXoutstr ( FILE *, char * );
extern void TeXoutsp ( FILE *, int );
extern int TeXReadToken ( char *, int * );
extern void TeXReadMacroName ( char * );
extern int TeXGetGenArg ( FILE *, char *, int, char, char, int );
extern int TeXGetArg ( FILE *, char *, int );
extern void TeXMustGetArg( FILE *, char *, int, char *, char * );
extern void TXnop ( TeXEntry * );
extern char *TXCopy ( char * );
extern void TXsavearg ( TeXEntry * );
extern void TXname ( TeXEntry * );
extern void TXraw ( TeXEntry * );
extern void TXcomment ( TeXEntry * );
extern void TXchar ( TeXEntry * );
extern void TXRemoveOptionalArg ( char * );
extern void TXRemoveOptionalStar ( char * );
extern void TXnopStar ( TeXEntry * );
extern void TXdoublebw ( TeXEntry * );
extern void TXtoday ( TeXEntry * );
extern void TXref ( TeXEntry * );
extern void TXnewline( TeXEntry * );
extern void TXlabel ( TeXEntry * );
extern void TXhref ( TeXEntry * );
extern void TXxref ( TeXEntry * );
extern void TXpxref ( TeXEntry * );
extern void TXmore ( TeXEntry * );
extern void PushCommentChar ( char );
extern void PopCommentChar (void);
extern void TXcode ( TeXEntry * );
extern void TXroutine ( TeXEntry * );
extern void TXdfn ( TeXEntry * );
extern void TXvar ( TeXEntry * );
extern void TXfile ( TeXEntry * );
extern void TXatletter ( TeXEntry * );
extern void TXatother ( TeXEntry * );
extern void TXasis ( TeXEntry * );
extern void TXasisGrouped ( TeXEntry * );
extern void TXbox ( TeXEntry * );
extern void TXinclude ( TeXEntry * );
extern void TXPopFile ( void );
extern void TXfileinclude ( TeXEntry * );
extern void TXIfFileExists( TeXEntry * );
extern void TXSetSplitLevel ( int, char * );
extern void TXsection ( TeXEntry * );
extern void TXtitlesection ( TeXEntry * );
extern void TXparagraph ( TeXEntry * );
extern void TXvtt ( TeXEntry * );
extern void TXvt ( TeXEntry * );
extern void TXdetails ( TeXEntry * );
extern void TeXskipEnv ( TeXEntry *, char *, int );
extern void TeXskipRaw ( TeXEntry *, char *, int );
extern void TXmathmode ( TeXEntry * );
extern void TeXBenign ( TeXEntry *, char * );
extern void TeXitemize ( TeXEntry * );
extern void TeXenumerate ( TeXEntry * );
extern void TeXdescription ( TeXEntry * );
extern void TeXDoList ( TeXEntry *, char *, char * );
extern void TeXsmall ( TeXEntry * );
extern void TeXiftex ( TeXEntry * );
extern void TeXtex ( TeXEntry * );
extern void TeXexample ( TeXEntry * );
extern void TXverb ( TeXEntry * );
extern void TeXverbatim ( TeXEntry * );
extern void TeXmenu ( TeXEntry * );
extern void TXbegin ( TeXEntry * );
extern void TXend ( TeXEntry * );
extern void TXitem ( TeXEntry * );
extern void TXcindex ( TeXEntry * );
extern void TXfindex ( TeXEntry * );
extern void TXindex ( TeXEntry * );
extern void TXprintindex ( TeXEntry * );
extern void TXmakeindex ( TeXEntry * );
extern void TXusepackage ( TeXEntry * );
extern void TXLoadPackage( const char * );
extern void TXdocumentstyle ( TeXEntry * );
extern void TXdocumentclass ( TeXEntry * );
extern void TXtitle ( TeXEntry * );
extern void TXauthor ( TeXEntry * );
extern void TXdate ( TeXEntry * );
extern void TeXmaketitle ( TeXEntry * );
extern void TXDef ( TeXEntry * );
extern void TXNewCommand ( TeXEntry * );
extern void TXNewLength ( TeXEntry * );
extern void TXURL ( TeXEntry * );
extern void TXAURL ( TeXEntry * );
extern void TXcite ( TeXEntry * );
extern void TXcatcode ( TeXEntry * );
extern void TXinitbreaktable ( void );
extern void TXoutactiveToken ( char * );
extern void TXepsfbox ( TeXEntry * );
extern void TXpsfig ( TeXEntry * );
extern void TXanimation ( TeXEntry * );
extern void TXInsertName ( SRList *, const char *, void (*)(TeXEntry *),
				     int, void * );
extern void TXInit ( FILE *, FILE * );
extern void TeXProcessCommand ( char *, FILE *, FILE * );
extern void TeXNoContents ( void );
extern char *ContentsLoc ( void );
extern void TeXWriteContents ( FILE * );
extern void ProcessLatexFile ( int, char **, FILE *, FILE * );
extern void TXSetCitePrefix ( char * );
extern void TXSetCiteSuffix ( char * );
extern void TXPrintToken ( FILE *, const char * );
extern int FileExists ( char * ); 
extern void TXincludegraphics( TeXEntry * );

/* tex2html.c */
extern void TXDoGaudy ( int );
extern void TXStartDoc ( int );
extern void TXpreformated ( FILE *, int );
extern void TeXoutNewline ( FILE * );
extern void TXbw ( TeXEntry * );
extern void TXbw2 ( TeXEntry * );
extern void TXoutbullet ( TeXEntry * );
extern void TXbf ( TeXEntry * );
extern void TXem ( TeXEntry * );
extern void TXsf ( TeXEntry * );
extern void TXrm ( TeXEntry * );
extern void TXbgroup ( TeXEntry * );
extern void TXegroup ( TeXEntry * );
extern void TXfont_tt ( TeXEntry * );
extern void TXfont_ss ( TeXEntry * );
extern void TXtt ( TeXEntry * );
/* extern void TXbitmap ( TeXEntry *, char * ); */
extern void TXWritePar ( FILE * );
extern void TXimage ( TeXEntry *, char * );
extern void TXInlineImage ( TeXEntry *, char * );
extern void TXAnchoredImage ( TeXEntry *, char *, char * );
extern void TXAnchoredInlineImage ( TeXEntry *, char *, char * );
extern void TXmovie ( TeXEntry *, char *, char *, char * );
extern void TXbbrace ( TeXEntry * );
extern void TXebrace ( TeXEntry * );
extern void TXmath ( TeXEntry * );
extern void TXmathend ( TeXEntry * );
extern void TXinlinemath ( TeXEntry * );
extern void TXinlinemathend ( TeXEntry * );
extern void TXWriteStartNewLine ( FILE * );
extern void TXWriteStartItem ( FILE * );
extern void TXmaketitle ( TeXEntry *, char *, char * );
extern void TXbitemize ( TeXEntry * );
extern void TXeitemize ( TeXEntry * );
extern void TXbenumerate ( TeXEntry * );
extern void TXeenumerate ( TeXEntry * );
extern void TXbdescription ( TeXEntry * );
extern void TXedescription ( TeXEntry * );
extern void TXbmenu ( FILE * );
extern void TXemenu ( FILE * );
extern void TXbdesItem ( TeXEntry * );
extern void TXedesItem ( TeXEntry * );
extern void TXWriteHyperLink ( FILE *, char *, char *, int );
extern void TXbcenter( FILE * );
extern void TXecenter( FILE * );

/* tohtml.c - should be moved */
extern void RemoveFonts ( const char *, char * );
extern void GetBaseName ( char * );
extern int SCHTMLTranslate ( char *, int );
extern void WriteHeader ( FILE * );
extern void WriteTrailer ( FILE * );
extern void WriteSectionHeader ( FILE *, char *, char *, int, char *, int );
extern void WriteSectionAnchor ( FILE *, char *, char *, int, int );
extern void WriteFileTitle ( FILE *, char * );
extern void WriteJumpDestination ( FILE *, char *, char * );
extern void WriteTextHeader ( FILE * );
extern void WritePopupTextReference ( FILE *, char *, char *, int );
extern void WritePointerText ( FILE *, char *, char *, int );
extern void WriteEndofTopic ( FILE * );
extern int SCHMTLTranslate ( char *, int );
extern int SCHTMLTranslateTables ( char *, int );
extern void WriteStartNewLine ( FILE * );
extern void WriteString ( FILE *, char * );
extern void WriteStringRaw ( FILE *, char * );
extern void WriteSectionButtons ( FILE *, char *, LINK * );
extern void WriteSectionButtonsBottom ( FILE *, char *, LINK * );
extern void OutJump ( FILE *, char *, char *, char * );
extern void SetUpButton ( FILE *, char *, char * );
extern void SetNextButton ( FILE *, char *, char * );
extern void SetPreviousButton ( FILE *, char *, char * );
extern void WritePar ( FILE * );
extern void WriteBeginPointerMenu ( FILE * );
extern void WriteEndOfPointer ( FILE * );
extern void WriteCenter ( FILE * );
extern void WriteLeftAligned ( FILE * );
extern void WriteRightAligned ( FILE * );
extern void WriteJustified ( FILE * );
extern void WritePlain ( FILE * );
extern void WriteRightIndent ( FILE *, int );
extern void WriteLeftIndent ( FILE *, int );
extern void WriteFirstLineIndent ( FILE *, int );
extern void WriteEndPage ( FILE * );
extern void WriteBeginPage ( FILE * );
extern void WriteHeadPage ( FILE * );

/* userdef.c */
extern void TXDebugDef ( int );
extern void TXDoUser ( TeXEntry * );
extern void TXAddUserDef ( SRList *, TeXEntry * );
extern void TXDoNewCommand ( SRList *, TeXEntry * );
extern void TXDoNewLength ( SRList *, TeXEntry * );
extern void TXlet ( TeXEntry * );
extern void TXDumpUserDefs ( FILE *, int );
extern void *TXCreateDefn( int nargs, char *ldef, int is_html );
extern void TXAddSkipFile( const char * );
extern void TXAddReplaceFile( const char *, const char * );
extern int  TXIsSkipFile( const char * );
extern int  TXIsReplaceFile( const char *, char * );

#ifdef __MSDOS__
#define MAX_TOKEN 512
#else
#define MAX_TOKEN 8*4096
#endif

#endif
