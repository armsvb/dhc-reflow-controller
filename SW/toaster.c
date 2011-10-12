///toaster.c

#include <Mega32.h>
#include <delay.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h> #define begin {
#define end }
#define map_size 128*64/8
#define max_time 340
#define base_temp 0
#define room_temp 40
//Timing
#define t1 30
#define t2 1000
//Keypads
#define maxkeys 13 //characters
#define no 11
#define yes 12
#define quit 13
#define ERR_SIZE 11
//Debouncing
#define release 1
#define debounce 2
#define stillPressed 3
#define debounceRelease 4
//Program modes
#define program1 0
#define program2 1
#define program3 2
#define preheat 3
#define ready 4
#define reflow 5
#define cooldown 6
#define done 7
#define abort 8
#define usedefault 9
void intialize();
void keyScan();
void buttonHandle();
void heat();
void pid();
eeprom int tempinput[max_time];
int timeinput, temp,timer,prevtemp=0;
char numinputs;
int err[ERR_SIZE];
char lcdmap[map_size];
int time1, time2, time3;
unsigned char key, butnum, maybe, scanState; //Non-debounced key press
int mode, prevmode;
char digits[10], index, str[32];
unsigned int number = 0;
#include "flash.c"
#include "lcd.c"
//===============================================
//timer 0 compare ISR
interrupt [TIM0_COMP] void timer0_compare(void)
{
//Decrement the three times if they are not already zero
if (time1>0) --time1;
if ((time2>0) && (mode==preheat || mode==ready ||mode==reflow||mode==abort||mode==cooldown)) --time2;
}
//===============================================
void interpolate(void)
{
int i=1,j,k=0;
int lastN=0;
float grad;
while(i<max_time)
{
if(tempinput[i] != 0)
{
grad = ((int)(tempinput[i]-tempinput[lastN]))/(((float)i-(float)lastN));
for(j=lastN+1,k=1; j<i ; j++,k++) tempinput[j] = (k)*grad + tempinput[lastN];
lastN = i;
}
i++;
}
}
//===============================================
int main (void)
{
intialize();

while(1)
{
if(time1 <= 0) keyScan();
if(time2 <= 0) heat();
}
}
//===============================================
void intialize()
{
int i=0, j=0;
for(i=0;i<map_size;i++) lcdmap[i]=0;
for(i=0;i<max_time;i++) tempinput[i]=0;
clear();
pixelxy(10,13,1); pixelxy(20,13,1); pixelxy(30,13,1); pixelxy(40,13,1);
pixelxy(51,34,1); pixelxy(51,54,1); pixelxy(51,74,1); pixelxy(51,94,1); pixelxy(51,114,1);
drawline(0,14,49,14);
drawline(50,14,50,127);

//Temperature labels
sprintf(str,"200"); putstr(8,0,str);
sprintf(str,"150"); putstr(18,0,str);
sprintf(str,"100"); putstr(28,0,str);
sprintf(str," 50"); putstr(38,0,str);
//Time labels
sprintf(str,"60"); putstr(53,30,str);
sprintf(str,"120"); putstr(53,48,str);
sprintf(str,"180"); putstr(53,68,str);
sprintf(str,"240"); putstr(53,88,str);
sprintf(str,"300"); putstr(53,108,str);

//default values

tempinput[0]=90; //Or room temp
tempinput[90]=150;
tempinput[180]=180;
//tempinput[210]=225;
tempinput[240]=225;
tempinput[270]=225;
timeinput=270;
numinputs=5;
interpolate();
scanState = release;
time1 = t1;
time2 = t2;
time3 = t3;

mode=usedefault;

sprintf(str,"Y:USE DEFAULT CURVE N:PROGRAM");
message(str);

for(i = 0; i < 10; i++) digits[i] = 0;
index = 0;

//set up timer 0
TIMSK = 2; //turn on timer 0 cmp match ISR
OCR0 = 250; //set the compare re to 250 time ticks
//prescalar to 64 and turn on clear-on-match
TCCR0=0b00001011;

DDRA = 0x00; //PORTA input
//adc stuff
ADMUX = 0b01100000;
//enable ADC and set prescaler to 1/128*16MHz=125,000
//and clear interupt enable
//and start a conversion
ADCSR = 0b11000111;

//crank up the ISRs
#asm
sei
#endasm
}
//===============================================
//Task subroutines
//keyScan: scan the key
void keyScan()
{
//Reset timeout counter
time1=t1;
//get lower nibble
DDRC = 0x0f;
PORTC = 0xf0;
delay_us(5);
key = PINC;

//get upper nibble
DDRC = 0xf0;
PORTC = 0x0f;
delay_us(5);
key = key | PINC;

//find matching keycode in keytbl
if (key != 0xff)
{
for (butnum=0; butnum<maxkeys; butnum++)
{
if (keytbl[butnum]==key) break;
}
if (butnum==maxkeys) butnum=0;
else butnum++; //adjust by one to make range 1-16
}
else butnum=0;

//debouncing finite state machine
switch(scanState)
{
case release:
if(0x00==butnum)
scanState=release;
else
{
scanState=debounce;
maybe=butnum;
}
break;
case debounce:
if(maybe==butnum)
{
scanState=stillPressed;
buttonHandle();
}
else
scanState=release;
break;
case stillPressed:
if(butnum==maybe)
scanState=stillPressed;
else
scanState=debounceRelease;
break;
case debounceRelease:
if(butnum==maybe)
scanState=stillPressed;
else
scanState=release;
break;
}
}
//===============================================
void buttonHandle()
{
int i;
if((butnum<=10) && (index<10))
{
digits[index++] = butnum - 1 + '0';
digits[index] = NULL;
number=atoi(digits);
if(mode == program1) sprintf(str,"ENTER TIME %d:%d",numinputs,number);
if(mode == program2) sprintf(str,"ENTER TEMPERATURE %d:%dC",numinputs,number);
message(str);
}

if(butnum == no)
{
for(i = 0; i < 10; i++) digits[i] = 0;
number=atoi(digits);
index = 0;
if(mode == 9/*usedefault*/)
{
//PORTA=0x00;
for(i=0;i<max_time;i++) tempinput[i]=0;
timeinput=0;
numinputs=0;
sprintf(str,"ENTER TEMPERATURE:");
mode = program2;
}
else if(mode == program1) sprintf(str,"ENTER TIME %d:%d",numinputs,number);
else if(mode == program2) sprintf(str,"ENTER TEMPERATURE %d:%dC",numinputs,number);
else if(mode == program3)
{
sprintf(str,"PROGRAMMED CURVE:PREHEATING...%dC",temp);
mode = preheat;
}
message(str);
}

if(butnum == yes)
{
if(mode==usedefault)
{
//timeinput=300;
numinputs=6;
drawgraph();
sprintf(str,"DEFAULT CURVE:PREHEATING...%dC",temp);
message(str);
mode = preheat;
}
else if(mode==program1)
{
if(number <= timeinput){
for(i = 0; i < 10; i++) digits[i] = 0;
index = 0;
sprintf(str,"ERROR:MUST BE MORE THAN %d!",timeinput);
message(str);
}
else if(number > max_time){
for(i = 0; i < 10; i++) digits[i] = 0;
index = 0;
sprintf(str,"ERROR:MUST BE LESS THAN %3d!",max_time);
message(str);
}
else{
timeinput = number;
mode = program2;

for(i = 0; i < 10; i++) digits[i] = 0;
number=atoi(digits);
index = 0;
sprintf(str,"ENTER TEMPERATURE %d:", numinputs);
message(str);
}
}
else if(mode==program2)
{
if(number > 250){
for(i = 0; i < 10; i++) digits[i] = 0;
index = 0;
sprintf(str,"ERROR:MUST BE 250C OR LESS!");
message(str);
}
else{
tempinput[timeinput] = number;
mode = program3;
numinputs++;
interpolate();
drawgraph();
sprintf(str,"Y:ADD NEXT POINT N:NO MORE",timeinput,number);
message(str);
}
}
else if(mode==program3)
{
mode = program1;
for(i = 0; i < 10; i++) digits[i] = 0;
number=atoi(digits);
index = 0;

sprintf(str,"ENTER TIME %d:", numinputs);
message(str);
}
else if(mode==ready)
{
timer=0;
ICR1H = 0xf4;
ICR1L = 0x24;
DDRD = 0xff; //PORTD output
TCCR1A = 0b10001110;
TCCR1B = 0b00011100;
OCR1A = 0x0000;
mode=reflow;
}
else if(mode==done)
{
intialize();
}

}
if(butnum==quit)
{
//PORTD.7^=1;
OCR1A = 0;
mode=abort;

sprintf(str,"ABORTING...");
message(str);
}
}
//===============================================
void heat()
{
int adch, adcl;
adcl = ADCL; //Read ADCL first; causes ADC block
adch = ADCH; //Read ADCH; remove the ADC block

adch = adch<<2; //Bits 9:2
adcl = adcl>>6; //Bits 1:0
adch |= adcl; //Index for conversion table
time2 = t2;

prevtemp = temp;
temp = temperature[adch];
ADCSR.6=1;
timer++;

if(mode == preheat){
if (temp < tempinput[0]){
PORTD.5=1;//PORTA = 0x00;// heater on
sprintf(str,"PREHEATING...WAIT:%d C",temp);
message(str);
}
else {
PORTD.5=0;
mode = ready;
cleargraph();
sprintf(str,"READY. INSERT PART AND PRESS Y");
message(str);
}
}
else if(mode==reflow) {
if(timer==timeinput) {
//heater off
//PORTD.7=0;
OCR1A = 0x0000;
mode=cooldown;
sprintf(str,"COOLING DOWN... TEMP:%3dC",temp);
message(str);
}
else{
pid();
//OCR1A = 0x0000;
//if(temp < tempinput[timer])PORTD.5=1;
//else PORTD.5=0;
sprintf(str,"TIME:%3d TMP:%3dC CRV-TMP:%3dC",timer,temp,tempinput[timer]);
message(str);
setcurvepixel(temp,timer);
}
}
else if(mode==cooldown) {
if(temp<=room_temp) {
mode=done;
sprintf(str,"DONE COOLING.REMOVE PART-PRESS Y");
}
else sprintf(str,"COOLING DOWN... TEMP:%3dC",temp);
message(str);
}
else if(mode==abort){
//PORTD.7=0;
// OCR1A = 0x0000;
mode=cooldown;
}
}
void pid()
{
long Kp = 12500; //6250; //Proportional constant
long Kd = 5000;//5000; //Derivative constant
long Ki = 0; //Integral constant
long Kf = 2000; //Future integral error;
long proportional, derivative, integral, futureintegral;
long dutycycle;
char i;

//Keep record of past 10 errors
for(i = ERR_SIZE-1; i > 0; i--){
err[i] = err[i-1];
}

//Error now
err[0] = tempinput[timer] - temp;

proportional = err[0];
derivative = err[0] - err[1];
//Past 10 errors
//integral = 0;
//for(i = 1; i < ERR_SIZE; i++) integral += err[i];

//Look into 10 future errors
futureintegral = 0;
if(timer < (max_time-ERR_SIZE))
//for(i = 1; i < ERR_SIZE*2; i++) futureintegral += tempinput[timer+i] - (temp+i*(temp-prevtemp);
for(i = 1; i < ERR_SIZE; i++) futureintegral += (tempinput[timer+i] - (temp+i*(temp-prevtemp))) * (ERR_SIZE-i)/(ERR_SIZE);

dutycycle = Kp * proportional + Kd * derivative + Ki * integral + Kf * futureintegral;
if(dutycycle > 62500) dutycycle = 62500;
if(dutycycle < 0 || timer >= timeinput) dutycycle = 0;
OCR1A = dutycycle;

}
