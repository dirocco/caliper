// caliper.c Ken Greenebaum 8/2/2002
//
// Caliper will consecuatively execute a command allowing the user to 'tune' 
// the parameters by passing it commands via standard in.  The command in
// turn is passed on the command line using the venerable printf syntax.
// 
// usage: adjuster command [-i] [-t]
//   command - string in printf format where %d specifies where to place the
//             'adjusted' variable
//   i       - optional initial variable value (default==XXX)
//   t       - optional timeout between last change and command firing
//             (default==XXX)


main(int argc, char *argv[])
{
   int x;
   char car;

   for(x= 0; x < 5; x++)
      printf(argv[1], x);

   while(1) {
      car = getchar();
      printf("got a <%c>\n", car);
}
