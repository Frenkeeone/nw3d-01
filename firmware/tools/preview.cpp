// Nahledy obrazovek na PC: preklad ui.cpp s LovyanGFX, vystup RAW RGB565 (big-endian).
#include "../NW3D_Eyes/ui.h"
#include <cstdio>
#include <string>
static LGFX_Sprite S[2];
static void dump(const std::string& name){ for(int e=0;e<2;e++){ std::string f=name+"_"+std::to_string(e)+".raw"; FILE* o=fopen(f.c_str(),"wb"); fwrite(S[e].getBuffer(),2,240*240,o); fclose(o);} }
int main(){
  for(auto& s:S){ s.setColorDepth(16); s.createSprite(240,240); }
  ui::begin();
  ClockInfo c; c.valid=true; c.hour=14; c.minute=37; c.second=42; c.wday=2; c.day=10; c.month=11;
  PrintInfo p; p.configured=p.online=p.hasData=true; p.phase=Phase::Printing; p.percent=67; p.remainingMin=83; p.layer=128; p.totalLayers=210; p.nozzle=220;
  for(int e=0;e<2;e++) ui::print(S[e],e,p,c,0); dump("print");
  p.phase=Phase::Paused; for(int e=0;e<2;e++) ui::print(S[e],e,p,c,0); dump("pause");
  p.phase=Phase::Preparing; p.percent=0; p.remainingMin=-1; p.layer=-1; p.totalLayers=-1; for(int e=0;e<2;e++) ui::print(S[e],e,p,c,800); dump("prep");
  EyeAnim a; a.gx=-14; a.gy=-6; a.open=1; for(int e=0;e<2;e++) ui::eyes(S[e],e,a,Mood::Normal); dump("eyes");
  a.open=0.35f; a.gx=0; a.gy=8; for(int e=0;e<2;e++) ui::eyes(S[e],e,a,Mood::Normal); dump("blink");
  for(int e=0;e<2;e++) ui::clock(S[e],e,c); dump("clock");
  WeatherInfo w; w.valid=true; w.temp=7.6f; w.tmin=2.1f; w.tmax=9.4f; w.code=2; w.isDay=true; w.rainProb=40;
  for(int e=0;e<2;e++) ui::weather(S[e],e,w); dump("weather");
  w.code=63; w.temp=-3.2f; for(int e=0;e<2;e++) ui::weather(S[e],e,w); dump("weather_rain");
  w.code=95; for(int e=0;e<2;e++) ui::weather(S[e],e,w); dump("weather_storm");
  w.code=73; for(int e=0;e<2;e++) ui::weather(S[e],e,w); dump("weather_snow");
  for(int e=0;e<2;e++) ui::finished(S[e],e,"v 15:52"); dump("finished");
  for(int e=0;e<2;e++) ui::failed(S[e],e,0x0300400Cu); dump("failed");
  for(int e=0;e<2;e++) ui::boot(S[e],e,0,"1.0"); dump("boot");
  for(int e=0;e<2;e++) ui::message(S[e],e,ui::ICON_PRINTER,"Žádná data","zapni Developer mode","nebo použij CLOUD"); dump("msg");
  return 0;
}
