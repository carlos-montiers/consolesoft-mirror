/*
  bwpatch.c - Byte-wise Patch: modify a binary file.

  BwPatch is a complement to BwDiff.  BwDiff produces output based on the
  differences of two files; BwPatch uses that output to modify a file.

  Jason Hood, 28 December, 1998 to 2 January, 1999.
  Public Domain.

  990401: Added Tell: & Quote: options;
	  Use -f to specify the patch file;
	  Select a file to patch from the patch file;
	  Always apply/restore patches (previously, answering "No" would
	   ignore the patch).

  v1.00, 16 & 17 October, 2003:
    Tell statement will also stop the query.

  v1.10, 26 & 27 October, 2012:
    increase maximum length to 1023;
    work with BwDiff v1.10;
    recognise more than just hexadecimal bytes;
    place the line number first in messages.
*/

#define PVERS "1.10"
#define PDATE "27 October, 2012"


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <limits.h>
#include <float.h>
#include <errno.h>
#include <ctype.h>

#ifdef _MSC_VER
# include <sys/utime.h>
#else
# include <utime.h>
# ifdef __LCC__
#  define utime _utime
# endif
#endif

#ifdef __DJGPP__
void   __crt0_load_environment_file( char* dummy ) { }
char** __crt0_glob_function( char* dummy ) { return 0; }
#endif


int restore;				// Restore original bytes
int update;				// Update timestamp
int quiet;				// Warnings
int yes;				// Auto-answer prompts

char** files;				// List of files to patch
char*  pfiles;				// Was a file patched?
int    num_files;			// Number of files to patch

#define LINE_LEN 1023			// Maximum length of the line/token
char line[LINE_LEN + 1];		// Line from the patch file
int  line_no;				// Line number
int  literal;				// Is it a literal token?
int  keep_line; 			// Read the line again


void  patch_file( char* name );
void  process_patch( FILE* patch, FILE* file );
void  process_token( char* token, unsigned char* bytes, int* len );
char* read_line( FILE* file );
char* read_token( char* line, char* token );
char* read_literal( char* line, char* token );
void  show_help( void );

#define ignore( file ) \
  do { int ch_; do \
    ch_ = getc( file );\
  while (ch_ != '\n' && ch_ != EOF); } while (0)

#define STRING static const char
STRING zNoMemory[]    = "Not enough memory.\n";
STRING zUnknownOpt[]  = "Unknown option: %c\n";
STRING zMissingFile[] = "\"%s\" does not contain: ";
STRING zNoFile[]      = "%s: File not found\n";
STRING zRestoring[]   = "Restoring";
STRING zPatching[]    = "Patching";
STRING zELengths[]    = "%d: Lengths differ.\n";
STRING zEOut[]	      = "%d: Out of file, ignoring line.\n";
STRING zDiffer[]      = "%d: Bytes differ; %s anyway? ";
STRING zRestore[]     = "restore";
STRING zPatch[]       = "patch";
STRING zEbRange[]     = "%d: Value \"%s\" out of range (-128..255).\n";
STRING zEwRange[]     = "%d: Value \"%s\" out of range (-32768..65535).\n";
STRING zEiRange[]     = "%d: Value \"%s\" out of range (-2147483648..4294967295).\n";
STRING zEfRange[]     = "%d: Value \"%s\" out of range (+/- %g).\n";
STRING zEInvalidHex[] = "%d: Invalid hexadecimal number \"%.2s\".\n";
STRING zEInvalidNum[] = "%d: Invalid number \"%s\".\n";
STRING zETrunc[]      = "%d: Line truncated.\n";


int main( int argc, char* argv[] )
{
  char* name = "crack";                 // Name of the patch file
  int	j, k;

  if (argc > 1)
  {
    if (strcmp( argv[1], "/?" ) == 0 ||
	strcmp( argv[1], "-?" ) == 0 ||
	strcmp( argv[1], "--help" ) == 0)
    {
      show_help();
      return 0;
    }
    if (strcmp( argv[1], "--version" ) == 0)
    {
      puts( "Byte-wise Patch version " PVERS " (" PDATE ")." );
      return 0;
    }
  }

  files  = malloc( argc * sizeof(char*) );
  pfiles = calloc( argc, 1 );
  if (files == NULL || pfiles == NULL)
  {
    fputs( zNoMemory, stderr );
    return 1;
  }
  num_files = 0;

  for (j = 1; j < argc; ++j)
  {
    if (argv[j][0] == '-')
    {
      for (k = 1; argv[j][k]; ++k)
      {
	switch (argv[j][k])
	{
	  case 'r': restore = 1; break;
	  case 'u': update  = 1; break;
	  case 'q': quiet   = 1; break;
	  case 'y': yes     = 1; break;

	  case 'f': name = (argv[j][k+1] == '\0') ? argv[++j] : argv[j] + k+1;
		    goto break_for;

	  default: fprintf( stderr, zUnknownOpt, argv[j][k] );
		   return 1;
	}
      }
      break_for: ;
    }
    else
      files[num_files++] = argv[j];
  }

  patch_file( name );

  if (!quiet)
  {
    k = 0;
    for (j = 0; j < num_files; ++j)
    {
      if (!pfiles[j])
      {
	if (!k)
	  k = fprintf( stderr, zMissingFile, name );
	else
	  fprintf( stderr, "%*c", k, ' ' );
	fputs( files[j], stderr );
	putc( '\n', stderr );
      }
    }
  }

  return 0;
}


// Open the file named by name and search for the file(s) to be patched.
void patch_file( char* name )
{
  FILE* patch;				// Patch file
  FILE* file;				// File to be patched
  char	token[LINE_LEN + 1];
  char* rest;
  struct stat finfo;
  struct utimbuf tinfo;
  int j;

  if ((patch = fopen( name, "r" )) == NULL)
  {
    fprintf( stderr, zNoFile, name );
    exit( 1 );
  }

  while ((rest = read_line( patch )) != NULL)
  {
    rest = read_token( rest, token );
    if (stricmp( token, "File:" ) == 0)
    {
      rest = read_token( rest, token );
      if (num_files != 0)		// Search the list
      {
	for (j = 0; j < num_files; ++j)
	  if (stricmp( token, files[j] ) == 0)
	    break;
	if (j == num_files)		// Didn't find it
	  continue;
	pfiles[j] = 1;
      }
      if ((file = fopen( token, "r+b" )) == NULL)
      {
	if (!quiet)
	  fprintf( stderr, zNoFile, token );
      }
      else
      {
	if (!update)
	  stat( token, &finfo );
	process_patch( patch, file );
	fclose( file );
	if (!update)
	{
	  tinfo.actime	= finfo.st_atime;
	  tinfo.modtime = finfo.st_mtime;
	  utime( token, &tinfo );
	}
      }
    }
  }

  fclose( patch );
}


// Patch file given the information in patch.
void process_patch( FILE* patch, FILE* file )
{
  char	token[LINE_LEN + 1];
  char* rest;
  int	ask = 0, process = 1;
  char* bp;
  long	offset = 0;
  unsigned char new_byte[LINE_LEN / 2];
  unsigned char old_byte[LINE_LEN / 2];
  unsigned char cur_byte[LINE_LEN / 2];
  int	new_len, old_len;
  int	new_literal, old_literal;
  int	match_new, match_old;
  int	j;

  while ((rest = read_line( patch )) != NULL)
  {
    rest = read_token( rest, token );
    if (stricmp( token, "Tell:" ) == 0)
    {
      rest = read_token( rest, token );
      printf( "%s %s.\n", (restore) ? zRestoring : zPatching, token );
      ask = 0;
      continue;
    }
    if (stricmp( token, "Quote:" ) == 0)
    {
      rest = read_token( rest, token );
      puts( token );
      continue;
    }
    if (stricmp( token, "Ask:" ) == 0)
    {
      if (!yes)
      {
	ask = 1;
	rest = read_token( rest, token );
	printf( "%s? ", token );
	fflush( stdout );
	*line = getchar();
	if (*line != '\n')
	  ignore( stdin );
	process = (restore) ? (*line == 'y' || *line == 'Y')
			    : (*line != 'n' && *line != 'N');
      }
      continue;
    }
    if (stricmp( token, "NoAsk" ) == 0)
    {
      ask = 0;
      continue;
    }
    if (stricmp( token, "File:" ) == 0)
    {
      keep_line = 1;
      break;
    }

    if (!literal)
    {
      offset = strtol( token, &bp, 16 );
      if (*bp == ':')
      {
	fseek( file, offset, SEEK_SET );
	rest = read_token( rest, token );
      }
    }

    new_len = old_len = 0;
    do
    {
      if (literal == 2)
      {
	for (j = 0; token[j]; ++j)
	{
	  new_byte[new_len++] = token[j];
	  new_byte[new_len++] = '\0';
	}
      }
      else if (literal == 1)
      {
	for (j = 0; token[j]; ++j)
	  new_byte[new_len++] = token[j];
      }
      else
      {
	process_token( token, new_byte, &new_len );
      }
      new_literal = literal;
      rest = read_token( rest, token );
    } while (*token != '[' && *token != '\0');

    for (;;)
    {
      old_literal = literal;
      rest = read_token( rest, token );
      if (*token == ']' || *token == '\0')
	break;
      if (literal == 2)
      {
	for (j = 0; token[j]; ++j)
	{
	  old_byte[old_len++] = token[j];
	  old_byte[old_len++] = '\0';
	}
      }
      else if (literal == 1)
      {
	for (j = 0; token[j]; ++j)
	  old_byte[old_len++] = token[j];
      }
      else
      {
	process_token( token, old_byte, &old_len );
      }
    }

    // A wide string may imply an extra 00, so add it explicitly.
    if (new_len != old_len && new_literal != old_literal)
    {
      if (new_literal == 2)
	old_byte[old_len++] = '\0';
      else if (old_literal == 2)
	new_byte[new_len++] = '\0';
    }

    if (new_len != old_len)
      fprintf( stderr, zELengths, line_no );
    else
    {
      if ((int)fread( cur_byte, 1, old_len, file ) != old_len)
	fprintf( stderr, zEOut, line_no );
      else
      {
	match_old = (memcmp( old_byte, cur_byte, old_len ) == 0);
	match_new = (memcmp( new_byte, cur_byte, old_len ) == 0);
	j = 1;
	if (!match_old && !match_new)
	{
	  printf( zDiffer, line_no, (restore) ? zRestore : zPatch );
	  fflush( stdout );
	  *line = getchar();
	  if (*line != '\n')
	    ignore( stdin );
	  if (*line != 'y' && *line != 'Y')
	    j = 0;
	}
	if (j)
	{
	  fseek( file, -old_len, SEEK_CUR );
	  fwrite( (yes || !ask) ? ((restore) ? old_byte : new_byte)
		   /* (ask) */	:  (process) ? new_byte : old_byte,
		  1, old_len, file );
      	  fseek( file, 0, SEEK_CUR );  // fread can't immediately follow fwrite
	}
      }
    }
  }
}


void process_token( char* token, unsigned char* bytes, int* len )
{
  int base = 10;
  int last = strlen( token ) - 1;
  int stop;
  int size;
  union
  {
    long   i;
    float  f;
    double d;
  } val;
  char* end;
  int	ival;

#if ULONG_MAX > 4294967295
#define IRANGE val.i < -2147483648 || val.i > 4294967295
#else
#define IRANGE errno == ERANGE
#endif

  if (token[last] == 'x')
    base = 16, stop = --last;
  else if (token[last-1] == 'x')
    base = 16, stop = last - 1;
  else
    stop = last;

  switch (token[last])
  {
    case 'b':
      val.i = (*token == '-') ? strtol( token, &end, base)
			      : (long)strtoul( token, &end, base );
      if (val.i < -128 || val.i > 255)
      {
	fprintf( stderr, zEbRange, line_no, token );
	exit( 2 );
      }
      size = 1;
      break;
    case 'w':
      val.i = (*token == '-') ? strtol( token, &end, base )
			      : (long)strtoul( token, &end, base );
      if (val.i < -32768 || val.i > 65535)
      {
	fprintf( stderr, zEwRange, line_no, token );
	exit( 2 );
      }
      size = 2;
      break;
    case 'i':
      val.i = (*token == '-') ? strtol( token, &end, base )
			      : (long)strtoul( token, &end, base );
      if (IRANGE)
      {
	fprintf( stderr, zEiRange, line_no, token );
	exit( 2 );
      }
      size = 4;
      break;
    case 'f':
      val.d = strtod( token, &end );
      if (val.d < -FLT_MAX || val.d > FLT_MAX)
      {
	fprintf( stderr, zEfRange, line_no, token, FLT_MAX );
	exit( 2 );
      }
      val.f = (float)val.d;
      size = 4;
      break;
    case 'd':
      val.d = strtod( token, &end );
      if (errno == ERANGE)
      {
	fprintf( stderr, zEfRange, line_no, token, DBL_MAX );
	exit( 2 );
      }
      size = 8;
      break;
    default:
      if (base == 16)
      {
	char* hex = token + stop + 1;
	do
	{
	  *hex = '\0';
	  if (--hex != token)
	    --hex;
	  val.i = strtol( hex, &end, 16 );
	  if (*end != '\0')
	  {
	    fprintf( stderr, zEInvalidHex, line_no, hex );
	    exit( 2 );
	  }
	  bytes[(*len)++] = (unsigned char)val.i;
	} while (hex != token);
      }
      else
      {
	do
	{
	  if (!isxdigit( token[0] ) || !isxdigit( token[1] ))
	  {
	    fprintf( stderr, zEInvalidHex, line_no, token );
	    exit( 2 );
	  }
	  sscanf( token, "%2x", &ival );
	  bytes[(*len)++] = ival;
	  token += 2;
	} while (*token);
      }
      size = 0;
      break;
  }
  if (size)
  {
    if (end != token + stop)
    {
      fprintf( stderr, zEInvalidNum, line_no, token );
      exit( 2 );
    }
    bytes += *len;
    *len += size;
    end = (char*)&val;
    do *bytes++ = *end++; while (--size);
  }
}


// Read a line from a file, ignoring blank lines and comments.
// Return NULL for EOF, pointer to first non-space otherwise.
char* read_line( FILE* file )
{
  int ch, len;

  if (keep_line)
  {
    keep_line = 0;
    len = 1;		// Just something != 0
  }
  else
  {
    ++line_no;
    len = 0;
    while ((ch = getc( file )) != EOF)
    {
      if (len == 0)
      {
	if (ch == ' ' || ch == '\t')
	  continue;
	if (ch == '\n')
	{
	  ++line_no;
	  continue;
	}
	if (ch == '#')
	{
	  ignore( file );
	  ++line_no;
	  continue;
	}
      }
      if (ch == '\n')
	break;
      if (len == LINE_LEN)
      {
	fprintf( stderr, zETrunc, line_no );
	ignore( file );
	break;
      }
      line[len++] = ch;
    }
    line[len] = '\0';
  }

  return (len == 0) ? NULL : line;
}


// Read a token from the line and point to the next.
char* read_token( char* line, char* token )
{
  literal = 0;

  if (line == NULL)
    *token = '\0';
  else
  {
    if (*line == 'L' && line[1] == '"')
    {
      literal = 2;
      line = read_literal( line + 2, token );
    }
    else if (*line == '"')
    {
      literal = 1;
      line = read_literal( line + 1, token );
    }
    else
    {
      while (*line != ' ' && *line != '\t' && *line != '\0' && *line != '#')
	*token++ = *line++;
      *token = '\0';
    }
    while (*line == ' ' || *line == '\t')
      ++line;
    if (*line == '\0' || *line == '#')
      line = NULL;
  }
  return line;
}


// Read a literal from the line and point to the character after closing quote.
// Literals can contain quotes by doubling the quote ("Literal ""quotes""").
char* read_literal( char* line, char* token )
{
  while (*line != '\0')
  {
    if (*line == '"')
    {
      ++line;
      if (*line != '"')
	break;
    }
    *token++ = *line++;
  }
  *token = '\0';

  return line;
}


void show_help( void )
{
  puts(
"Byte-wise Patch by Jason Hood <jadoxa@yahoo.com.au>.\n"
"Version " PVERS " (" PDATE ").  Public Domain.\n"
"http://misc.adoxa.cjb.net/\n"
"\n"
"BwPatch is a complement to BwDiff.  BwDiff produces output based on the\n"
"differences of two files; BwPatch uses that output to modify a file.\n"
"\n"
"bwpatch [file...] [-f patch] [-qruy]\n"
"\n"
"  file    the file(s) to modify (defaults to all files in patch)\n"
"  patch   the file containing the patch(es) (defaults to \"crack\")\n"
"  -q      suppress missing file warning\n"
"  -r      restore the original bytes\n"
"  -u      update the timestamp\n"
"  -y      answer queries automatically\n"
"\n"
"Patch file format:\n"
"\n"
"# Comments extend to the end of the line.\n"
"File:  \"file name\"                     # file to be patched\n"
"Tell:  \"message\"                       # display message (see below)\n"
"Quote: \"Message.\"                      # display message\n"
"Ask:   \"Prompt\"                        # query the user (see below)\n"
"NoAsk                                  # stop the query\n"
"<offset>: <new>       [ <old> ]        # offset & bytes in hexadecimal\n"
"          123x        [ 0123x ]        # little-endian\n"
"          \"a string\"  [ L\"wide\" ]      # strings in quotes\n"
"          1b 2w 4i    [ 1bx 2xw 4xi ]  # [hexa]decimal byte, word or integer\n"
"          4f 8d       [ 4.0f 8.0d ]    # float, single and double precision\n"
"\n"
"The Tell message is prefixed with \"Patching \" or \"Restoring \" and\n"
"suffixed with \".\"; the Ask prompt is suffixed with \"? \".  The default\n"
"answer is yes when patching, no when restoring.  The answer will apply\n"
"to all patches up to the next Tell, Ask, NoAsk or File.  To include a\n"
"quote in a string, double it (\"a \"\"quoted\"\" string\")."
	);
}

