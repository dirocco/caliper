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
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
// #include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>


#include <math.h>

int parseARGV(char *commandLine, char **argv) {
   enum states { S_START, S_IN_TOKEN};
   int argc=0;
   int done=0;
   enum states state = S_START;
   
   while (!done) {
      switch (state) {
      case S_START:
	if (*commandLine==0)
	  done=1;
	if (*commandLine<=32)
	  *commandLine=0;
	else {
	  argv[argc++]=commandLine;
	  state=S_IN_TOKEN;
	}
	break;
      case S_IN_TOKEN:
	if (*commandLine==0)
	  done=0;
	if (*commandLine<=32) {
	  *commandLine=0;
	  state=S_START;
	}
	break;
      }
      commandLine++;
    }
   argv[argc]=(char*)0;  /* store the NULL pointer */
   return argc;
}


main(int argc, char *argv[])
{
   int i=0,x=0;
   char car;

   double delta = 1.0;
   
   struct {
      double value; 
      int fraction;
   } var[5] = { 0 };

   int index = 0;

   static struct termios Otty, Ntty;
   char string[100];

   while(++x < argc) {
      if (*argv[x] == '-') {
         switch(*++argv[x]) {
            case 'i':
               while (i < 5 && ++x < argc && *argv[x] != '-')
                   var[i++].value = atof(argv[x]);
               break;
            case 't':
               break;
         }
      }
   }

   sprintf(string, argv[1], 
	  var[0].value, var[1].value, var[2].value, var[3].value); 
   printf("%s (hit <space> to run)\n", string);

   fflush(stdout);
   tcgetattr( 0, &Otty);
   Ntty = Otty;

   Ntty.c_iflag      = 0;   // input mode
   Ntty.c_lflag      = 0;   // line settings (no echo)
   Ntty.c_cc[VMIN]   = 1;   // minimum time to wait
   Ntty.c_cc[VTIME]  = 1;   // minimum characters to wait for
   tcsetattr( 0, TCSANOW, &Ntty);

   pid_t pid = 0;
   int done = 0;
   while(!done) {
      car = getchar();

      if((car<'d')&&(car>='a')) {
	 index = car - 'a';
      } else if ((car<='9')&&(car>='0')) {
	 int tmp = car - '0';
	 // delta = pow(10.0, -1.0*tmp);
	 delta = pow(10.0, (var[index].fraction?1:-1)*tmp);
	 printf("tmp=%d, %f\n", tmp, delta);
	 putchar('\n');
      } else {
	 switch(car) {
	    case ' ':
	       printf(argv[1], 
		     var[0].value, var[1].value, var[2].value, var[3].value); 
	       printf("\n");
	    break;

	    case '=':
	    case '+': var[index].value+=delta; break;
	    case '-': var[index].value-=delta; break;

	    case 'q': done = 1; break;
	    case 'f': 
               var[index].fraction= !var[index].fraction; 
	    break;

	    case '?':
	    default: printf("got a <%c>\n", car); break;
	 }
      }
  
      sprintf(string, argv[1], 
	    var[0].value, var[1].value, var[2].value, var[3].value); 
      printf("%s (hit <space> to re-run)\n", string);

     if (car == ' ') {
#if 0
     system(string);
#else
     char *args[5];
     parseARGV(string, args);
        

     int status;
     if (pid) {
        kill(pid, SIGKILL);
        wait(&status);
     }
     if(!(pid = vfork())) {
	// printf("child\n");
	execvp(args[0], &args[0]);
     } else {
	// printf("parent\n");
     } 
     }
#endif

   }

   if (pid)
      kill(pid, SIGKILL);
   tcsetattr( 0, TCSANOW, &Otty);

   exit(0);
}
