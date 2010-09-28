#include <stdio.h>
#include <fcntl.h>
#include <X11/XWDFile.h>

main( argc, argv )
int argc;
char **argv;
{
int fd;
XWDFileHeader header;

fd = open( argv[1], O_RDONLY );
if (!fd) {
    fprintf( stderr, "Could not open %s\n", argv[1] );
    return 1;
    }

read( fd, &header, sizeof(header) );

printf( "%d %d\n", header.pixmap_width, header.pixmap_height );
return 0;
}
