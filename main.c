#include <lynx.h>
#include <6502.h>
#include <tgi.h>
#include <joystick.h>
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>

#include "bg.pal"
#include "levels.h"
#include "abc.h"

#define  A_BUTTON 1 
#define  B_BUTTON  2 
#define  LEFT_BUTTON 32 
#define  RIGHT_BUTTON 16 
#define  UP_BUTTON 128 
#define  DOWN_BUTTON 64 

extern void abcstop ();
extern void abcplay (unsigned char channel, char *tune);
extern unsigned char abcactive[4];
// Special low-level calls to set up underlying hardware
// abcoctave legal values 0..6
extern void __fastcall__ abcoctave(unsigned char chan, unsigned char val);
// abcpitch legal values 0..255
extern void __fastcall__ abcpitch(unsigned char chan, unsigned char val);
// abctaps legal values 0..511
extern void __fastcall__ abctaps(unsigned char chan, unsigned int val);
// abcintegrate legal values 0..1
extern void __fastcall__ abcintegrate(unsigned char chan, unsigned char val);
// abcvolume legal values 0..127
extern void __fastcall__ abcvolume(unsigned char chan, unsigned char val);

extern unsigned char bg[];
extern unsigned char logo[];
extern unsigned char rail0[];
extern unsigned char rail1[];
extern unsigned char box[];
extern unsigned char fail[];
extern unsigned char smallbox[];
extern unsigned char smallfail[];
extern unsigned char select[];
extern unsigned char elf[];
extern unsigned char calendar[];
extern unsigned char toys000000[];
extern unsigned char toys000001[];
extern unsigned char toys000002[];
extern unsigned char toys000003[];
extern unsigned char toys000004[];
extern unsigned char toys000005[];
extern unsigned char smalltoys000000[];
extern unsigned char smalltoys000001[];
extern unsigned char smalltoys000002[];
extern unsigned char smalltoys001000[];
extern unsigned char smalltoys001001[];
extern unsigned char smalltoys001002[];
extern unsigned char gears000000[];
extern unsigned char gears000001[];
extern unsigned char gears000002[];
extern unsigned char gears000003[];

unsigned char reset,paused,soundenabled;


unsigned char pos,posy,joy,keypressed,framecount,moving,falling,toypos,wait,curlevel;
signed int off,timer;
unsigned char txtbuf[21]; 
unsigned char nexttoys[13];
unsigned char nexttoy; 
unsigned char targets[18]; 
unsigned char numtarget;
unsigned char curtarget;
unsigned char fails;
unsigned int good;
unsigned char boxes[6];
unsigned char delay;
unsigned char lastempty;

unsigned char gear0,gear1,gear2,gear3;

unsigned char temppack;
unsigned char packcount;
unsigned char kind;
unsigned char bgdir;
signed int bgx,bgy;

SCB_REHV_PAL sp_bg= 
{
  BPP_4 | TYPE_BACKGROUND,
  REHV,
  0x01,
  0,
  bg,
  0, 0, 0x0100, 0x0100,
  { 0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef }
};

SCB_REHV_PAL sp_gfx= 
{
  BPP_4 | TYPE_NORMAL,
  REHV,
  0x01,
  0,
  box,
  0, 0, 0x0100, 0x0100,
  { 0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef }
};

SCB_REHV_PAL sp_cursor= 
{
  BPP_4 | TYPE_NORMAL,
  REHV,
  0x01,
  0,
  select,
  0, 0, 0x0100, 0x0100,
  { 0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef }
};

unsigned char level[28] = {0};
unsigned char emptyspace[3] = {0};

unsigned char * toy[7] = {emptyspace, toys000000, toys000001, toys000002, toys000003, toys000004, toys000005};
unsigned char * smalltoy[7] = {emptyspace, smalltoys000000, smalltoys000001, smalltoys000002, smalltoys001000, smalltoys001001, smalltoys001002};
unsigned char * gears[4] = {gears000000, gears000001, gears000002, gears000003};

unsigned char levelstate[36] = {0};

const char nop90[] = "A game by NOP90";
const char jam[] ="LynXmas 2020 GameJam";
const char opt1[] ="Puzzle";
const char opt2[] ="Arcade";

void initialize()
{
	tgi_install(&tgi_static_stddrv);
    tgi_init();
	tgi_clear();
	tgi_setframerate(60);
	joy_install(&joy_static_stddrv);
    CLI();
}

void drawbg(void)
{
	switch (bgdir)
	{
		case 3:
			bgx-=1;
			if(bgx==-103)
				bgx+=102;
			break;
		case 2:
			bgy+=1;
			if(bgy==1)
				bgy-=102;
			break;
		case 1:
			bgx+=1;
			if(bgx==1)
				bgx-=102;
			break;
		default:
			bgy-=1;
			if(bgy==-103)
				bgy+=102;
			break;
	}
	sp_bg.vpos=bgy;
	sp_bg.hpos=bgx;
	tgi_sprite(&sp_bg);
	sp_bg.hpos+=102;
	tgi_sprite(&sp_bg);
	sp_bg.hpos+=102;
	tgi_sprite(&sp_bg);
	sp_bg.vpos+=102;
	tgi_sprite(&sp_bg);
	sp_bg.hpos-=102;
	tgi_sprite(&sp_bg);
	sp_bg.hpos-=102;
	tgi_sprite(&sp_bg);
}

unsigned char menu(void)
{
	unsigned char done=0;
	unsigned char sel=0;
	unsigned char framecount=0;
	keypressed=1;
	abcplay (0, tune0_A);
	abcplay (1, tune0_B);
	abcplay (2, tune0_C);
  
	while (!done)
    {
		joy = joy_read(JOY_1);
		if (!joy) keypressed = 0;
		if(joy&A_BUTTON && !keypressed)
		{
			abcplay (3, sfx_2);
			done=1;	
		}
		if(joy&RIGHT_BUTTON && !keypressed)
		{
			abcplay (3, sfx_0);
			bgdir=3;
			keypressed=1;
		}
		if(joy&LEFT_BUTTON && !keypressed)
		{
			abcplay (3, sfx_0);
			bgdir=1;
			keypressed=1;
		}
		if(joy&DOWN_BUTTON && !keypressed)
		{
			abcplay (3, sfx_0);
			bgdir=0;
			sel=(sel+1)&1;
			keypressed=1;
			framecount=32;	
		}
		if(joy&UP_BUTTON && !keypressed)
		{
			abcplay (3, sfx_0);
			bgdir=2;
			sel=(sel+1)&1;
			keypressed=1;
			framecount=32;	
		}
		drawbg();
		sp_gfx.data=logo;
		sp_gfx.hpos=0;
		sp_gfx.vpos=1;
		tgi_sprite(&sp_gfx);
		tgi_setcolor(1);
		tgi_outtextxy(23,84,nop90);
		tgi_outtextxy(1,94,jam);
		tgi_setcolor(15);
		tgi_outtextxy(22,83,nop90);
		tgi_outtextxy(0,93,jam);
		tgi_setcolor(15);
		tgi_outtextxy(53,41,opt1);
		tgi_outtextxy(53,61,opt2);
		if(++framecount&32 && sel==0)
			tgi_setcolor(2);
		else	
			tgi_setcolor(3);
		tgi_outtextxy(52,40,opt1);
		if(++framecount&32 && sel==1)
			tgi_setcolor(2);
		else	
			tgi_setcolor(3);
		tgi_outtextxy(52,60,opt2);
		tgi_updatedisplay();
		while (tgi_busy());
		tgi_setpalette(bg_pal);
		rand();
	}
	return sel+1;
}
void genlevel(void)
{
	unsigned char i;
	delay=3;
	for (i=0;i<28;++i)
		level[i]=rand()%6+1;
	for(i=0;i<6;++i)
	{
		boxes[i]=0;
		targets[i*3]=rand()%6+1;
		targets[i*3+1]=rand()%6+1;
		targets[i*3+2]=rand()%6+1;
	}
	good=0;
	fails=0;
	numtarget=1;
}

void loadlevel(unsigned char lev)
{
	unsigned char i;
	delay=levels[lev][0];
	for (i=0;i<28;++i)
		level[i]=levels[lev][i+1];
	numtarget=levels[lev][29];
	for(i=0;i<6;++i)
	{
		boxes[i]=0;
		targets[i*3]=levels[lev][30+i*3];
		targets[i*3+1]=levels[lev][31+i*3];
		targets[i*3+2]=levels[lev][32+i*3];
	}

	for(i=0;i<13;++i)
	{
		nexttoys[i]=levels[lev][48+i];
	}

	curtarget=0;
	nexttoy=0;
	fails=0;
}

void drawGears(unsigned char s)
{
	sp_gfx.vpos=90;
	sp_gfx.data=gears[s%4];
	sp_gfx.hpos=15;
	tgi_sprite(&sp_gfx);
	sp_gfx.data=gears[(s+1)%4];
	sp_gfx.hpos=46;
	tgi_sprite(&sp_gfx);
	sp_gfx.data=gears[(s+2)%4];
	sp_gfx.hpos=76;
	tgi_sprite(&sp_gfx);
	sp_gfx.data=gears[(s+2)%4];
	sp_gfx.hpos=107;
	tgi_sprite(&sp_gfx);
	sp_gfx.data=gears[(s+3)%4];
	sp_gfx.hpos=137;
	tgi_sprite(&sp_gfx);
	sp_gfx.vpos=63;
	sp_gfx.data=gears[(164-s)%4];
	sp_gfx.hpos=15;
	tgi_sprite(&sp_gfx);
	sp_gfx.data=gears[(163-s)%4];
	sp_gfx.hpos=45;
	tgi_sprite(&sp_gfx);
	sp_gfx.data=gears[(162-s)%4];
	sp_gfx.hpos=75;
	tgi_sprite(&sp_gfx);
	sp_gfx.data=gears[(161-s)%4];
	sp_gfx.hpos=105;
	tgi_sprite(&sp_gfx);
	sp_gfx.data=gears[(160-s)%4];
	sp_gfx.hpos=136;
	tgi_sprite(&sp_gfx);
	sp_gfx.vpos=36;
	sp_gfx.data=gears[s%4];
	sp_gfx.hpos=153-15;
	tgi_sprite(&sp_gfx);
	sp_gfx.data=gears[(s+1)%4];
	sp_gfx.hpos=153-45;
	tgi_sprite(&sp_gfx);
	sp_gfx.data=gears[(s+2)%4];
	sp_gfx.hpos=153-75;
	tgi_sprite(&sp_gfx);
	sp_gfx.data=gears[(s+2)%4];
	sp_gfx.hpos=153-105;
	tgi_sprite(&sp_gfx);
	sp_gfx.data=gears[(s+3)%4];
	sp_gfx.hpos=153-136;
	tgi_sprite(&sp_gfx);
}

void screenin(void)
{
	unsigned char i;
	unsigned int s=160;
	while(s)
	{
		drawbg();
		sp_gfx.sprctl0 = BPP_4 | TYPE_NORMAL;	
		sp_gfx.data=rail0;
		sp_gfx.hpos=0;
		if(s>150)
			sp_gfx.vpos=89+s-150;
		else
			sp_gfx.vpos=89;
		tgi_sprite(&sp_gfx);
		sp_gfx.data=rail1;
		sp_gfx.hpos=-s;
		sp_gfx.vpos=62;
		tgi_sprite(&sp_gfx);
		sp_gfx.hpos=159+s;
		sp_gfx.vpos=35;
		sp_gfx.sprctl0 = BPP_4 | TYPE_NORMAL | HFLIP;	
		tgi_sprite(&sp_gfx);
		s-=2;
		tgi_updatedisplay();
		while (tgi_busy());
	} 
	s=160;
	while(s)
	{
		drawbg();
		sp_gfx.sprctl0 = BPP_4 | TYPE_NORMAL;	
		sp_gfx.data=rail0;
		sp_gfx.hpos=0;
		sp_gfx.vpos=89;
		tgi_sprite(&sp_gfx);
		sp_gfx.data=rail1;
		sp_gfx.vpos=62;
		tgi_sprite(&sp_gfx);
		sp_gfx.hpos=159;
		sp_gfx.vpos=35;
		sp_gfx.sprctl0 = BPP_4 | TYPE_NORMAL | HFLIP;	
		tgi_sprite(&sp_gfx);

		sp_gfx.sprctl0 = BPP_4 | TYPE_NORMAL;	
		sp_gfx.vpos=19;
		for(i=1;i<10;++i)
		{
			sp_gfx.data=toy[level[i]];
			sp_gfx.hpos=160-16*(i)+s;
			tgi_sprite(&sp_gfx);
		}
		sp_gfx.vpos=46;
		for(i=10;i<19;++i)
		{
			sp_gfx.data=toy[level[i]];
			sp_gfx.hpos=16*(i-10)-s;
			tgi_sprite(&sp_gfx);
		}
		sp_gfx.vpos=73;
		for(i=19;i<29;++i)
		{
			sp_gfx.data=toy[level[i%28]];
			sp_gfx.hpos=160-16*(i-18)+s;
			tgi_sprite(&sp_gfx);
		}
		drawGears(s);
		s-=2;
		tgi_updatedisplay();
		while (tgi_busy());
	} 
}

void drawscreen(signed int offset)
{
	unsigned char i;
	signed int t;
	
	drawbg();
	if(paused)
		tgi_outtextxy(36,50,"Game paused");
	else
	{
		sp_gfx.sprctl0 = BPP_4 | TYPE_NORMAL;	
		sp_gfx.data=rail0;
		sp_gfx.hpos=0;
		sp_gfx.vpos=89+offset;
		tgi_sprite(&sp_gfx);
		sp_gfx.data=rail1;
		sp_gfx.hpos=-offset;
		sp_gfx.vpos=62;
		tgi_sprite(&sp_gfx);
		sp_gfx.hpos=159+offset;
		sp_gfx.vpos=35;
		sp_gfx.sprctl0 = BPP_4 | TYPE_NORMAL | HFLIP;	
		tgi_sprite(&sp_gfx);
		sp_gfx.sprctl0 = BPP_4 | TYPE_NORMAL;	
		sp_gfx.vpos=19;
		for(i=0;i<10;++i)
		{
			sp_gfx.data=toy[level[(i+toypos)%28]];
			sp_gfx.hpos=176-16*(i+1)-off+offset;
			tgi_sprite(&sp_gfx);
		}
		sp_gfx.data=toy[level[(10+toypos)%28]];
		sp_gfx.vpos=46-falling;
		sp_gfx.hpos=off-offset;
		tgi_sprite(&sp_gfx);
		sp_gfx.vpos=46;
		for(i=11;i<19;++i)
		{
			sp_gfx.data=toy[level[(i+toypos)%28]];
			sp_gfx.hpos=16*(i-10)+off-offset;
			tgi_sprite(&sp_gfx);
		}
		sp_gfx.data=toy[level[(19+toypos)%28]];
		sp_gfx.vpos=73-falling+offset;
		sp_gfx.hpos=160-16-off;
		tgi_sprite(&sp_gfx);
		sp_gfx.vpos=73+offset;
		for(i=20;i<(29-lastempty);++i)
		{
			sp_gfx.data=toy[level[(i+toypos)%28]];
			sp_gfx.hpos=160-16*(i-18)-off;
			tgi_sprite(&sp_gfx);
		}

		if(off && !offset)
			drawGears(off);

		for(i=0;i<numtarget;++i)
		{
			sp_gfx.hpos=20*i;
			sp_gfx.vpos=1;
			if(boxes[i])
			{
				if(boxes[i]==1)
					sp_gfx.data=smallbox;
				else
					sp_gfx.data=smallfail;
				tgi_sprite(&sp_gfx);
			}
			else
			{
				sp_gfx.data=smalltoy[targets[i*3]];
				tgi_sprite(&sp_gfx);
				sp_gfx.hpos+=4;
				sp_gfx.vpos+=4;
				sp_gfx.data=smalltoy[targets[i*3+1]];
				tgi_sprite(&sp_gfx);
				sp_gfx.hpos+=4;
				sp_gfx.vpos+=4;
				sp_gfx.data=smalltoy[targets[i*3+2]];
				tgi_sprite(&sp_gfx);
			}
		}

		if(kind)
		{
			sp_gfx.vpos=1;
			sp_gfx.data=smallfail;
			for(i=0;i<fails;++i)
			{
				sp_gfx.hpos=25 + 20*i;
				tgi_sprite(&sp_gfx);
			}
			sp_gfx.data=smallbox;
			sp_gfx.hpos=70;
			tgi_sprite(&sp_gfx);
			tgi_setcolor(2);
			sprintf(txtbuf,"%03d",good);
			tgi_outtextxy(90,5,txtbuf);
		}

		if(packcount)
		{
			if(packcount>16)
				t=32-packcount;
			else
				t=packcount;
			sp_gfx.hsize=t<<4;
			sp_gfx.vsize=t<<4;

			sp_gfx.hpos=16*pos+8-t;
			sp_gfx.vpos=56-t;
			
			if(temppack)
				sp_gfx.data=fail;
			else
				sp_gfx.data=box;
			tgi_sprite(&sp_gfx);
				
		}
		sp_gfx.hsize=0x100;
		sp_gfx.vsize=0x100;

		if(!offset)
		{
			if(posy)
			{
				sp_cursor.vpos=99-posy*27;
				sp_cursor.hpos=16*pos-1-16;
				tgi_sprite(&sp_cursor);
				sp_cursor.hpos+=16;
				tgi_sprite(&sp_cursor);
				sp_cursor.hpos+=16;
				tgi_sprite(&sp_cursor);
			}
			else
			{
				sp_cursor.hpos=16*pos-1;
				sp_cursor.vpos=17;
				tgi_sprite(&sp_cursor);
				sp_cursor.vpos=44;
				tgi_sprite(&sp_cursor);
				sp_cursor.vpos=71;
				tgi_sprite(&sp_cursor);
			}
			tgi_setcolor(3);
			sprintf(txtbuf,"%02d",timer);
			tgi_outtextxy(142,5,txtbuf);
		}
	}
	tgi_updatedisplay();
	while (tgi_busy());
	sp_cursor.penpal[1]=(sp_cursor.penpal[1]+1)&15;
}

unsigned char checkpack(unsigned char g1, unsigned char g2, unsigned char g3)
{
	unsigned char g=7;
	unsigned char s=7;
	unsigned char i;
	for (i=0;i<3;++i)
	{
		if((g & (1)) && (s & (1<<i)))
			if(g1==targets[curtarget*3+i])
			{
				g=g&6;
				s=s&(7-1<<i);	
			}
		if((g & (2)) && (s & (1<<i)))
			if(g2==targets[curtarget*3+i])
			{
				g=g&5;
				s=s&(7-1<<i);	
			}
		if((g & (4)) && (s & (1<<i)))
			if(g3==targets[curtarget*3+i])
			{
				g=g&3;
				s=s&(7-1<<i);	
			}
	}
	if(g || s)
		return 1;
	return 0;	
}

void levelIntro(void)
{
	unsigned char done=0;
	unsigned char cleared=0;
	unsigned passed=0;
	unsigned char i,temp;
	keypressed=1;
	for(i=0;i<curlevel;++i)
	{
		if(levelstate[i]==1)
			++passed;
		if(levelstate[i]==2)
			++cleared;
	}
	while(!done && !reset)
	{
		joy = joy_read(JOY_1);
		if (!joy) keypressed = 0;
		if(kbhit())
		{
			temp=cgetc();
			if(temp=='R')
				reset=1;
		}
		if(joy&A_BUTTON && !keypressed)
		{
			done=1;
		}
		drawbg();
		sp_gfx.data=elf;
		sp_gfx.hpos=5;
		sp_gfx.vpos=20;
		tgi_sprite(&sp_gfx);
		sp_gfx.data=calendar;
		sp_gfx.hpos=140;
		sp_gfx.vpos=1;
		tgi_sprite(&sp_gfx);
		sprintf(txtbuf,"%i",curlevel+8);
		tgi_setcolor(3);
		if(curlevel<2)
			tgi_outtextxy(145,2,txtbuf);
		else	
			tgi_outtextxy(141,2,txtbuf);
		
		tgi_setcolor(15);
		sprintf(txtbuf,"Level %02d",curlevel+1);
		tgi_outtextxy(74,28,txtbuf);
		if(curlevel)
		{
			tgi_setcolor(3);
			tgi_outtextxy(86,48,"Score");
			sprintf(txtbuf,"%02d Cleared",cleared);
			tgi_outtextxy(68,58,txtbuf);
			if(passed)
			{
				sprintf(txtbuf,"%02d Passed",passed);
				tgi_outtextxy(72,68,txtbuf);
			}
		}
		tgi_setcolor(15);
		tgi_outtextxy(16,94,"Press A to start");
		tgi_updatedisplay();
		while (tgi_busy());
	}
}

void gameEnd(void)
{
	unsigned char done=0;
	unsigned char cleared=0;
	unsigned passed=0;
	unsigned char i,temp;
	keypressed=1;
	for(i=0;i<curlevel;++i)
	{
		if(levelstate[i]==1)
			++passed;
		if(levelstate[i]==2)
			++cleared;
	}
	while(!done && !reset)
	{
		joy = joy_read(JOY_1);
		if (!joy) keypressed = 0;
		if(kbhit())
		{
			temp=cgetc();
			if(temp=='R')
				reset=1;
		}
		if(joy&A_BUTTON && !keypressed)
		{
			done=1;
		}
		drawbg();
		sp_gfx.data=elf;
		sp_gfx.hpos=5;
		sp_gfx.vpos=20;
		tgi_sprite(&sp_gfx);
		sp_gfx.data=calendar;
		sp_gfx.hpos=140;
		sp_gfx.vpos=1;
		tgi_sprite(&sp_gfx);
		tgi_setcolor(3);
			tgi_outtextxy(141,2,"25");
		tgi_setcolor(15);
		sprintf(txtbuf,"Game completed",curlevel+1);
		tgi_outtextxy(48,20,txtbuf);
		if(curlevel)
		{
			tgi_setcolor(3);
			tgi_outtextxy(86,40,"Score");
			sprintf(txtbuf,"%02d Cleared",cleared);
			tgi_outtextxy(68,50,txtbuf);
			if(cleared==16)
				tgi_outtextxy(78,60,"Perfect score");
			else
				sprintf(txtbuf,"%02d Passed",passed);
			tgi_outtextxy(72,60,txtbuf);
		}
		tgi_setcolor(15);
		tgi_outtextxy(52,94,"Press A");
		tgi_updatedisplay();
		while (tgi_busy());
	}
}


unsigned char game(void)
{
	unsigned char done=0;
	unsigned char temp,i;
	signed int exitcount;
	pos=5;
	posy=0;
	timer=99;
	framecount=0;
	moving=0;
	off=0;
	toypos=0;
	falling=0;
	wait=3;
	packcount=0;
	
	keypressed=1;
	if(kind)
		genlevel();
	else
		loadlevel(curlevel);
  
	if(!reset)
		screenin();
	while (!done && !reset)
    {
		joy = joy_read(JOY_1);
		if (!joy) keypressed = 0;
		if(kbhit())
		{
			temp=cgetc();
			if(temp=='R')
				reset=1;
			if(temp=='P')
				paused=1-paused;
		}
		if(!paused)
		{
			if(joy&LEFT_BUTTON && !keypressed)
			{
				abcplay (3, sfx_0);
				bgdir=1;
				if(pos>1)
					--pos;
				keypressed=1;	
			}
			if(joy&RIGHT_BUTTON && !keypressed)
			{
				abcplay (3, sfx_0);
				bgdir=3;
				if(pos<8)
					++pos;
				keypressed=1;	
			}
			if(joy&A_BUTTON && !keypressed && !moving && !packcount)
			{
				if(posy==3)
				{
					temppack=checkpack(level[(9-pos+toypos)%28],level[(10-pos+toypos)%28],level[(11-pos+toypos)%28]);
					level[(9-pos+toypos)%28]=0;
					level[(10-pos+toypos)%28]=0;
					level[(11-pos+toypos)%28]=0;
				}
				else if(posy==2)
				{
					temppack=checkpack(level[(9+pos+toypos)%28],level[(10+pos+toypos)%28],level[(11+pos+toypos)%28]);
					level[(9+pos+toypos)%28]=0;
					level[(10+pos+toypos)%28]=0;
					level[(11+pos+toypos)%28]=0;
				}
				else if(posy==1)
				{
					temppack=checkpack(level[(27-pos+toypos)%28],level[(28-pos+toypos)%28],level[(29-pos+toypos)%28]);
					level[(27-pos+toypos)%28]=0;
					level[(28-pos+toypos)%28]=0;
					level[(29-pos+toypos)%28]=0;
				}
				else
				{
					temppack=checkpack(level[(10-pos+toypos)%28],level[(10+pos+toypos)%28],level[(28-pos+toypos)%28]);
					level[(10-pos+toypos)%28]=0;
					level[(10+pos+toypos)%28]=0;
					level[(28-pos+toypos)%28]=0;
				}
				if(temppack)
					abcplay (3, sfx_4);
				else
					abcplay (3, sfx_5);
				packcount=32;
				keypressed=1;	
			}
			if(joy&B_BUTTON && !keypressed)
			{
				if(posy)
				{
					abcplay (3, sfx_2);
					posy=0;
				}	
				else
				{
					abcplay (3, sfx_3);
					posy=2;
				}
				keypressed=1;	
			}
			if(joy&UP_BUTTON && !keypressed && !moving && !packcount)
			{
				bgdir=2;
				if(posy)
				{
					abcplay (3, sfx_0);
					if(posy<3)
						++posy;
				}
				else
				{
					abcplay (3, sfx_6);
					temp=level[(10-pos+toypos)%28];
					level[(10-pos+toypos)%28]=level[(10+pos+toypos)%28];
					level[(10+pos+toypos)%28]=temp;
				}
				keypressed=1;	
			}
			if(joy&DOWN_BUTTON && !keypressed && !moving && !packcount)
			{
				bgdir=0;
				if(posy)
				{
					abcplay (3, sfx_0);
					if(posy>1)
						--posy;
				}
				else
				{
					abcplay (3, sfx_6);
					temp=level[(10+pos+toypos)%28];
					level[(10+pos+toypos)%28]=level[(28-pos+toypos)%28];
					level[(28-pos+toypos)%28]=temp;
				}
				keypressed=1;	
			}
			if(posy)
			{
				if(pos==1)
					++pos;
				if(pos==8)
					--pos;
			}
		}
		drawscreen(0);
		if(!paused)
		{
			if(falling)
				falling-=3;
			++framecount;
			if(framecount==30)
			{
				framecount=0;
				if(timer)
					--timer;
				--wait;
			}
			if(!wait)
			{
				moving=1;
				wait=delay;
				if(level[toypos]==0)
				{
					lastempty=1;
					if(!kind)
					{
						level[toypos]=nexttoys[nexttoy];
						nexttoy=(nexttoy+1)%13;
					}
				}
				if(kind)
					level[toypos]=rand()%6+1;
			}	
			if(moving)
				off+=2;
			if(off>16)
			{
				off=0;
				moving=0;
				lastempty=0;
				toypos=(toypos+27)%28;
				falling=21;
			}
			if(packcount==1)
			{
				if(kind)
				{
					timer+=30;
					if (timer>99)
						timer=99;
					if(!temppack)
					{
						++good;	
						if(good==15)
							delay=2;
						if(good==30)
							delay=1;
					}
					for(i=0;i<5;++i)
					{
						targets[i*3]=targets[i*3+3];
						targets[i*3+1]=targets[i*3+4];
						targets[i*3+2]=targets[i*3+5];
					}
					targets[15]=rand()%6+1;
					targets[16]=rand()%6+1;
					targets[17]=rand()%6+1;
				}
				else
				{
					boxes[curtarget]=temppack+1;
					++curtarget;
				}
				if(temppack)
					++fails;
			}
			if(packcount)
				--packcount;
			else
			{
				if(fails>2 || fails>=numtarget)
					done=2;
				else if (curtarget>=numtarget && !kind)
				{
					if(!fails)
						levelstate[curlevel]=2;
					else	
						levelstate[curlevel]=1;
					done=3;
				}
				else if(!timer && !packcount)
					done=1;
			}
		}
	}
	if(!reset)
		for(exitcount=0;exitcount<161;exitcount+=2)
			drawscreen(exitcount);
	return done;
}

void main(void)
{
	unsigned char sel,res,done,i,temp;
	initialize();
	bgdir=2;
	bgx=0;
	bgy=0;
	gear0=0;
	gear1=1;
	gear2=2;
	gear3=3;
	soundenabled=1;
	while (1)
    {
		reset=0;
		paused=0;
		sel = menu();

		curlevel=0;
		kind=sel-1;
		abcplay (0, tune1_A);
		abcplay (1, tune1_B);
		abcplay (2, tune1_C);
		for(i=0;i<21;++i)
			levelstate[i]=0;
		while(sel && !reset)
		{
			if(sel==1)
				levelIntro();
			res=game();
			done=0;
			if(sel==2)
				sprintf(txtbuf,"%03d",good);
			keypressed=1;
			while(!done && !reset)
			{
				drawbg();
				if(sel==2)
				{
					sp_gfx.data=logo;
					sp_gfx.hpos=0;
					sp_gfx.vpos=1;
					tgi_sprite(&sp_gfx);
					tgi_setcolor(15);
					tgi_outtextxy(24,35,"Your score is:");
					tgi_setcolor(3);
					tgi_outtextxy(68,60,txtbuf);
					tgi_setcolor(15);
					tgi_outtextxy(0,90,"A:Play Again  B:Menu");
				}
				else
				{
					if(res==3)
					{
						if(!fails)
						{
							sprintf(txtbuf,"Level %02d cleared",curlevel+1);
							tgi_outtextxy(16,10,txtbuf);
							tgi_setcolor(15);
							tgi_outtextxy(0,90,"Press A for next lvl");
							sp_gfx.data=box;
							sp_gfx.hpos=40;
							sp_gfx.vpos=30;
							tgi_sprite(&sp_gfx);
							sp_gfx.hpos+=48;
							tgi_sprite(&sp_gfx);
						}
						else
						{
							sprintf(txtbuf,"Level %02d passed",curlevel+1);
							tgi_outtextxy(20,10,txtbuf);
							tgi_outtextxy(16,68,"But not cleared!");
							tgi_setcolor(15);
							tgi_outtextxy(0,90,"A:Next  B:Play again");
							sp_gfx.data=box;
							sp_gfx.hpos=40;
							sp_gfx.vpos=30;
							tgi_sprite(&sp_gfx);
							sp_gfx.data=fail;
							sp_gfx.hpos+=48;
							tgi_sprite(&sp_gfx);
						}
					}
					else
					{
						if(res==2)
						{
							tgi_outtextxy(32,10,"Level failed");
						}
						else	
							tgi_outtextxy(44,10,"Time over");
						sp_gfx.data=fail;
						sp_gfx.hpos=40;
						sp_gfx.vpos=30;
						tgi_sprite(&sp_gfx);
						sp_gfx.hpos+=48;
						tgi_sprite(&sp_gfx);
						tgi_setcolor(15);
						tgi_outtextxy(0,90,"Press A to try again");
					}
				}
				tgi_updatedisplay();
				while (tgi_busy());
				joy = joy_read(JOY_1);
				if (!joy) keypressed = 0;
				if(kbhit())
				{
					temp=cgetc();
					if(temp=='R')
						reset=1;
				}
				if(joy&A_BUTTON && !keypressed)
				{
					done=1;
					if(res==3)
						++curlevel;
				}
				if(joy&B_BUTTON && !keypressed)
				{
					if(fails)
						done=1;
					if(sel==2)
					{
						sel=0;
						done=1;
					}
				}
			}
			if(curlevel>15)
			{
				gameEnd();
				sel=0;
			}
		}
	}
}
