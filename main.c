
#ifndef ARDUINO

extern void setup();
extern void loop();

int main()
{
  
  setup();
  
  while(1)
  {
    loop();
  }
  return 0;
}
#endif

