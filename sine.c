#include <math.h>     // sin()
#include "pablio.h"

#define SAMPLE_RATE  44100
#define FREQUENCY      440.0
#define NUM_FRAMES    1000

float buffer[NUM_FRAMES];

int main(int argc, char *argv[])
{
   PABLIO_Stream  *outStream;
   double theta = 0.0, delta = 2.0 * 3.1415 / (double)SAMPLE_RATE;
   double frequency = 1.0, attenuation = 1.0;
   int x; 

   if (argc > 1)
      frequency = atof(argv[1]);
   if (argc > 2)
      attenuation = atof(argv[2]);

   OpenAudioStream(&outStream, SAMPLE_RATE, paFloat32, PABLIO_WRITE|PABLIO_MONO);

   while(1) {  // synthesize and output samples forever
      for(x= 0; x<NUM_FRAMES; x++) { // synthesize a buffer full of samples
         buffer[x] = attenuation * sin(theta); // ugly, I know...
         theta += frequency * delta;
      }

      // blocking write provides flow control
      WriteAudioStream(outStream, buffer, NUM_FRAMES); 
   }
}
