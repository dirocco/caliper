// caliper.c Ken Greenebaum 8/2/2002
//
// Caliper will consecuatively execute a command allowing the user to 'tune' 
// the parameters by passing it commands via standard in.  The command in
// turn is passed on the command line using the venerable printf syntax.
// 
// usage: adjuster command [-i] [-t]
//   command - string in printf format where %f specifies where to place the
//             'adjusted' variable
//   i       - optional initial variable value (default==XXX)
//   t       - optional timeout between last change and command firing
//             (default==XXX)

#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/types.h>



#include <math.h>
// #include <curses.h>

main(int argc, char *argv[])
{
   int x;
   char car;

   double delta = 1.0;
   double value[5];
   int index = 0;

   static struct termios Otty, Ntty;

   fflush(stdout);
   tcgetattr( 0, &Otty);
   Ntty = Otty;

   Ntty.c_iflag      = 0;   // input mode
   Ntty.c_lflag      = 0;   // line settings (no echo)
   Ntty.c_cc[VMIN]   = 1;   // minimum time to wait
   Ntty.c_cc[VTIME]  = 1;   // minimum characters to wait for
   tcsetattr( 0, TCSANOW, &Ntty);

   int done = 0;
   while(!done) {
      car = getchar();

      if((car<'d')&&(car>='a')) {
	 index = car - 'a';
      } else if ((car<='9')&&(car>='0')) {
	 int tmp = car - '0';
	 delta = pow(10.0, -1.0*tmp);
	 printf("tmp=%d, %f\n", tmp, delta);
	 putchar('\n');
      } else {
	 switch(car) {
	    case ' ':
	       printf(argv[1], value[0], value[1], value[2], value[3]); 
	    break;

	    case '+': value[index]+=delta; break;
	    case '-': value[index]-=delta; break;

	    case 'q': done = 1; break;

	    default: printf("got a <%c>\n", car); break;
	 }
      }
  
   printf(argv[1], value[0], value[1], value[2], value[3]); 
   putchar('\n');
   }

//tcsetattr(0, FLAG, &Otty);   
}
