
#ifndef ARDUINO

extern void setup();
extern void loop();

volatile int doLoop = 1;

int main()
{
  
  setup();
  
  while(doLoop)
  {
    loop();
  }
  return 0;
}
#endif

