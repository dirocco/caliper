// caliper.c Ken Greenebaum 8/2/2002
//
// Caliper will consecuatively execute a command allowing the user to 'tune' 
// the parameters by passing it commands via standard in.  The command in
// turn is passed on the command line using the venerable printf syntax.
// 
// usage: adjuster command [-i] [-t]
//   command - string in printf format where %f specifies where to place the
//             'adjusted' variable
//   i       - optional initial variable value list
//   f       - optional initial variable fraction list
//   m       - optional magnitude variable list
//   t       - optional timeout between last change and command firing
//             (default==XXX)

void usage() {
   printf("usage: caliper command [-i] [-t]\n");
   printf("       command - string in printf format where %%f specifies\n");
   printf("                 where to place the 'adjusted' variable\n");
   printf("       i - optional initial variable value list \n");
   printf("       f - optional fraction list\n");
   printf("       m - optional magnitude list\n");
   //printf("       t - optional timeout between last change and command firing\n");
}

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

void help() {
   printf("a-d: set parameter to modify\n");
   printf("0-9: set delta (power of 10)\n");
   printf("+,-: increment and decrement\n");
   printf("f:   toggle fractional delta\n");
   printf("z:   zero current parameter\n");
   printf("i:   restore initial parameter\n");
   printf("!:   toggle immediate re-run\n");
   printf("?:   help\n");
   printf("q:   quit\n");
}

main(int argc, char *argv[])
{
   int i, x;
   char car;
   int done;
   char *string;

   double delta;
   
   struct {
      double value; 
      double initial; 
      int fraction;
      int magnitude;
   } var[5] = { 0 };

   int index     = 0;
   int immediate = 1;

   static struct termios Otty, Ntty;

   for(x = 0; x < 5; x++)
      var[x].initial = 0.0;

   {
   int arg;
   char *string;
   char state = 'l';

   for(arg = 1; arg < argc; arg++) {
      string = argv[arg];

      switch(state) {
         case 'l':
	    if(*string == '-') {
	       switch(string[1]) {
		  case 'f': state='f'; i = 0; break;
		  case 'i': state='i'; i = 0; break;
		  case 'm': state='m'; i = 0; break;
		  case 't': break;

		  default: 
                     usage(); exit(-1);
	       }
	    }
	 break;

	 case 'f':
	    if(*string != '-') {
	       var[i++].fraction = atoi(string);
	    } else {
               arg--;
               state = 'l';
	    }
	 break;

	 case 'm':
	    if(*string != '-') {
	       var[i++].magnitude = atoi(string);
	    } else {
               arg--;
               state = 'l';
	    }
	 break;

	 case 'i':
	    if(*string!='-') {
	       var[i].initial = atof(string);
	       var[i].value   = var[i].initial;
               i++;
	    } else {
               arg--;
               state = 'l';
	    }
	 break;
      }
   } // while
   }


   help();

   delta = pow(10.0, (var[index].fraction?-1:1)*var[index].magnitude);
   string = (char *) malloc(200);
   sprintf(string, argv[1], 
	  var[0].value, var[1].value, var[2].value, var[3].value); 
   printf("%s delta = %f (hit <space> to run)\n", string, delta);

   fflush(stdout);
   tcgetattr( 0, &Otty);
   Ntty = Otty;

   Ntty.c_iflag      = 0;   // input mode
   Ntty.c_lflag      = 0;   // line settings (no echo)
   Ntty.c_cc[VMIN]   = 1;   // minimum time to wait
   Ntty.c_cc[VTIME]  = 1;   // minimum characters to wait for
   tcsetattr( 0, TCSANOW, &Ntty);

   pid_t pid = 0;
   done = 0;
   while(!done) {
      car = getchar();

      if((car<'d')&&(car>='a')) {
	 index = car - 'a';
      } else if ((car<='9')&&(car>='0')) {
         var[index].magnitude = car - '0';
	 putchar('\n');
      } else {
	 switch(car) {
	    case ' ': break;

	    case '=':
	    case '+': var[index].value+=delta; break;
	    case '-': var[index].value-=delta; break;

	    case 'i': var[index].value=var[index].initial; break;
	    case 'z': var[index].value=0.0; break;

	    case 'q': done = 1; break;
	    case 'f': 
               var[index].fraction= !var[index].fraction; 
               delta = 1.0/delta;
	    break;

	    case '!': immediate = !immediate; break;
	    case '?': help(); break;

	    default: printf("got a <%c>\n", car); break;
	 }
      }
  
      delta = pow(10.0, (var[index].fraction?-1:1)*var[index].magnitude);
      sprintf(string, argv[1], 
	    var[0].value, var[1].value, var[2].value, var[3].value); 
      printf("%s delta = %f", string, delta);
      if (!immediate)
         printf(" (hit <space> to re-run)");
      printf("\n");

     if (immediate || car == ' ') {
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
