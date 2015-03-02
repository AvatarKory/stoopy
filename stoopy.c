/*
 * stoopy: Really Stupid and Dumb Terminal Program
 * Copyright (C) 2005-2015, Kory Hamzeh, Avatar Consultants, Inc.
 *
 * This program is licensed under the GNU General Public License V2.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <termios.h>
#include <unistd.h>

#define VERSION "1.02"

typedef struct {
  int	rate;
  int	code;
} BAUD_RATE_CODE;

char *my_name;
char *com_port_name;
int  com_port_fd;
int slow_write = 0;
int slow_delay = 50; /* milliseconds */
int baud_rate = B115200;
int char_size = CS8;
int hw_flow_control = 0;

BAUD_RATE_CODE baud_rates[] = { 
  { 1200,   B1200  }, { 2400,   B2400   }, { 4800,   B4800  }, 
  { 9600,   B9600  }, { 19200,  B19200  }, { 38400,  B38400 }, 
  { 57600,  B57600 }, { 115200, B115200 }, { -1,     -1     } };

#define USAGE \
  "\n"									\
  "Really Stupid and Dump Terminal Program, Version " VERSION "\n"	\
  "\n"									\
  "usage: stoopy [-h] [-s] [-d millisec-delay] [-b baud-rate]\n"	\
  "       [-7] [-o | -e] [-f] com-port-device-name\n"			\
  "\n"									\
  "  -h         Print this message.\n"					\
  "  -s         Enable slow write mode.\n"				\
  "  -d delay   Set the slow write inter character delay time. Default\n" \
  "             is 50 milliseconds.\n"					\
  "  -b rate    Set baud rate to one of: 1200, 2400, 4800, 9600, 19200, \n" \
  "             38400, 57600, 115200. Default is 115200 BPS.\n"		\
  "  -7         Select 7 bits per character (defqult is 8).\n"		\
  "  -e         Select even parity.\n"					\
  "  -o         Select odd parity.\n"					\
  "  -f         Enable hardware flow control (off by default).\n"	\
  "\n"									\
  "Default setting is 115200 baud, 8-N-1.\n\n"

int usage()
{
  fprintf( stderr, USAGE );
  exit( 1 );
}

int main( int argc, char *argv[] )
{
  fd_set		rset;
  int			done = 0;
  int			cnt;
  char			buf[ 256 ];
  int			rc;
  int			i;
  struct termios 	options;
  long			baud;
  long			save_lflags;
  long			parity = 0;
  int			arg_ndx;
  int			bits = CS8;

  my_name = argv[ 0 ];

  arg_ndx = 1;

  while( ( arg_ndx < argc ) && ( argv[ arg_ndx ][ 0 ] == '-' ) ) {
    switch( argv[ arg_ndx ][ 1 ] )
      {
      case 'h':
	usage();
	break;

      case 's':
	slow_write = 1;
	break;

      case 'f':
	hw_flow_control = 1;
	break;

      case 'd':
	++arg_ndx;
	if( arg_ndx >= argc ) usage();
	slow_delay = atoi( argv[ arg_ndx ] );
	break;

      case 'b':
	++arg_ndx;
	if( arg_ndx >= argc ) usage();
	baud = atoi( argv[ arg_ndx ] );
	i = 0;
	while( baud_rates[ i ].code != -1 ) {
	  if( baud_rates[i].rate == baud ) {
	    baud_rate = baud_rates[ i ].code;
	    break;
	  }
	  ++i;
	}

	if( baud_rates[ i ].code == -1 ) {
	  fprintf( stderr, "\ninvalid baud rate\n" );
	  usage();
	}
	break;

      case '7':
	bits = CS7;
	break;

      case 'e':
	parity = PARENB;
	break;

      case 'o':
	parity = ( PARENB | PARODD );
	break;

      default:
	usage();
	exit( 1 );
      }

    ++arg_ndx;
  }

  if( ( argc - arg_ndx ) != 1 ) {
    usage();
  }

  com_port_name = argv[ arg_ndx ];
  
  if( ( com_port_fd = open( com_port_name, O_RDWR | O_NOCTTY ) ) < 0 ) {
    fprintf( stderr, "can't open %s\n", com_port_name );
    exit( 1 );
  }
  
  /*
   * Get the current options for the port...
   */
  
  tcgetattr( com_port_fd, &options );
  
  /*
   * Set the baud rates
   */
  
  cfsetispeed( &options, baud_rate );
  cfsetospeed( &options, baud_rate );
  
  /*
   * Enable the receiver and set local mode...
   */
  
  options.c_cflag |= ( CLOCAL | CREAD );
  
  /* Set to 8 bits, no parity, 1 stop bit */ 
  
  options.c_cflag &= ~CSIZE; /* Mask the character size bits */

  options.c_cflag &= ~PARENB;
  options.c_cflag &= ~CSTOPB;
  options.c_cflag &= ~CSIZE;
  options.c_cflag |= ( bits | parity );

  /* Enable/Disable hardware flow control */
  if( hw_flow_control ) {
    options.c_cflag |= CRTSCTS;
  }
  else {
    options.c_cflag &= ~CRTSCTS;
  }

  options.c_iflag = IGNPAR;
         
  /* Raw output.  */
  options.c_oflag = 0;
         
  /* disable all echo functionality, and don't send signals 
     to calling program */
  options.c_lflag = 0;

  /* Disable all control charactor processing */
  memset( &options.c_cc, 0, sizeof( options.c_cc ) );
 
  /*
   * Set the new options for the port...
   */

  tcflush( com_port_fd, TCIFLUSH );
  tcsetattr( com_port_fd, TCSANOW, &options );

  /* Disable local echo */
  tcgetattr( STDIN_FILENO, &options );
  save_lflags = options.c_lflag;
  options.c_lflag &= ~( ICANON | ECHO | ECHOE | ISIG );
  tcsetattr( STDIN_FILENO, TCSANOW, &options );
 
  printf( "\nHit Control-D to exit this program.\n\n" );
 
  while( !done ) {
    
    FD_ZERO( &rset );
    FD_SET( com_port_fd, &rset );
    FD_SET( STDIN_FILENO, &rset ); /* stdin */
   
    rc = select( com_port_fd + 1, &rset, NULL, NULL, NULL );
    
    if( rc < 0 ) {
      done = 1;
      continue;
    }
    
    if( FD_ISSET( STDIN_FILENO, &rset ) ) {
      cnt = read( STDIN_FILENO, buf, sizeof( buf ) );
      if( cnt > 0 && buf[ 0 ] == 4 ) {
	done = 1;
	continue;
      }
      if( cnt > 0 ) {

	if( slow_write ) {
	  i = 0;
	  while( cnt > i ) {
	    write( com_port_fd, &buf[i], 1 );
	    usleep( slow_delay * 1000 );
	    i++;
	  }
	}
	else {
	  write( com_port_fd, buf, cnt );
	}
      }
    }
    
    if( FD_ISSET(com_port_fd, &rset ) ) {
      cnt = read( com_port_fd, buf, sizeof( buf ) );
      if( cnt > 0 ) {
	write( STDOUT_FILENO, buf, cnt );
      }
    }
  }
 
  /* Close the com port */ 
  close( com_port_fd );
 
  /* Reset stdin to what it was when we started */ 
  tcgetattr( STDIN_FILENO, &options );
  options.c_lflag = save_lflags;
  tcsetattr( STDIN_FILENO, TCSANOW, &options );

  printf( "\nExiting .... \n" );

  exit( 0 );
}

