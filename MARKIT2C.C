/**************
 ** MarkIT2C **
 **************/

/******** Include header files *******/

#include "cap2.h"         /* Included in all CAP programs */

#include "interfac.h"    /* System manager includes for interface */
#include "event.h"       /* and event types */
#include "cougraph.h"

#include <string.h>

#include "lstring.h"	 /* Include long string functions */
#include "chtype.h"
#include "dosfile.h"
#include "dtn_edit.h"
#include "sysdefs.h"

#include "dosfile.h"

#ifdef TKERNEL
#include "chktsr.c"
#endif


#define  TITLE_HEIGHT 10

#define STACK_TOP     14
#define STACK_BOTTOM 174
#define STACK_LEFT     0 /* 112 */

char Target[80],Search[80],Dir[80],Answer[80];

char far *msgTestApp="MarkIt2C";
char far *msgAppTopLine="MarkIt Markdown Editor";

char far *fkeyQuit="Quit";
char far *menuHex="New\tF1";
char far *menuDec="Open\tF2";
char far *menuOct="Save\tF3";
char far *menuBin="Save As\tF4";
char far *menuBase="File";
char far *menuQuit="&Quit";

char far *msgENVName="C:\\_DAT\\MARKIT2C.ENV";

char far *msgNull="";

char far **StringTable[]={
&msgTestApp,
&msgAppTopLine,
&fkeyQuit,
&menuQuit,
&msgENVName,
&msgNull,
&menuQuit,
};





char cursordata[]={
0,0,0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00,
    0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00,
    0xff,0xff, 0xff,0xff, 0xff,0xff, 0xff,0xff, 0xff,0xff, 0xff,0xff,
    0xff,0xff, 0xff,0xff, 0xff,0xff, 0xff,0xff, 0xff,0xff, 0xff,0xff,
    0xff,0xff, 0xff,0xff, 0xff,0xff, 0xff,0xff, 0xff,0xff, 0xff,0xff,
    0xff,0xff, 0xff,0xff,
    0xff,0xff, 0xff,0xff, 0xff,0xff, 0xff,0xff, 0xff,0xff, 0xff,0xff,
    0xff,0xff, 0xff,0xff, 0xff,0xff, 0xff,0xff, 0xff,0xff, 0xff,0xff,
    0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00,
    0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00,
    0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00,
    0x00,0x00, 0x00,0x00};


/**************** Handlers *******************/
int far MyCardHandler(PWINDOW Wnd, WORD Message, WORD Data, WORD Extra);

void far DoQuit(void);

void Uninitialize(void);


#define STACK_DEPTH 80
#define STACK_HEIGHT 40


/******** Global state data *******/
EVENT app_event;          /* System manager event struct */
CAPBLOCK CapData;         /* CAP application data block */


/* Stuff we save in the .ENV */
/*long int*/
char Stack[STACK_DEPTH*STACK_HEIGHT];
int depth=0;
int Base=10;
char Log[STACK_DEPTH] = "log\0";


BOOL Done;                /* Global flag for program termination */
int err;
long int temp;
int startnumber=1;

FKEY MyFKeys[]= {
 {  &fkeyQuit,	 DoQuit,	    1,		0 },
 {  &fkeyQuit,	 DoQuit,	    2,		 0 },
 {  &fkeyQuit,	 DoQuit,	    3,		 0 },
 {  &fkeyQuit,	 DoQuit,	    4,		 0 },
 {  &fkeyQuit,	 DoQuit,	    5,		 0 },
 {  &fkeyQuit,	 DoQuit,	    6,		 0 },
 {  &fkeyQuit,	 DoQuit,	    7,		 0 },
 {  &fkeyQuit,	 DoQuit,	    8,		 0 },
 {  &fkeyQuit,	 DoQuit,	    9,		 0 },
 {  &fkeyQuit,	 DoQuit,	   10+LAST_FKEY, 0 }
};

/******* Menu structures *******/
// Reminder of MENU structure:  (for complete details, refer to CAP.H)
// MENU = { {Title, Handler, HotKey, Style}, ...}
// End of menu indicated by null record.

MENU BaseMenu[] = {
 { &menuHex,	   DoQuit, 0 ,0, NO_HELP},
 { &menuDec,	   DoQuit, 0 ,0, NO_HELP},
 { &menuOct,	   DoQuit, 0 ,0, NO_HELP},
 { &menuBin,	   DoQuit, 0 ,0, NO_HELP},
 { 0, 0, 0, 0}
};

/**** TopMenu "hangs" all the previous menus off itself with MENU_PULLDOWN ****/
MENU TopMenu[] = {
 { &menuBase,      (PFUNC) BaseMenu,     0, MENU_PULLDOWN },
  { &menuQuit,	    DoQuit,		  0 },
 { 0, 0, 0, 0}
};


/*** MyCard Dialog Window ***/
WINDOW MyCard={MyCardHandler,
       0,0,640,190,
       &msgAppTopLine,0,
       0,0,
       NULL,MyFKeys,TopMenu,NO_HELP};



/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
/******                                                                ******/
/******                END OF STRUCTURES--CODE BELOW                   ******/
/******                                                                ******/
/****************************************************************************/
/****************************************************************************/
/****************************************************************************/


void FixupFarPtrs(void)
{
  int i;
  int dataseg;

  _asm {
    mov ax,ds
    mov dataseg,ax
  }

  for (i=0; i<countof(StringTable); i++)
      *(((int *)(StringTable[i]))+1) = dataseg;
}



void far DoBeep(void)
{
  /*m_beep();*/
}


void far DoQuit(void)
{
  Done = TRUE;
}

void CreateMainView(void)
{
  SendMsg(&MyCard, CREATE, CREATE_FOCUS, 0);
}


char *FormatNum(unsigned long int num)
{
  static char buffer[33];
  char *p=buffer;
  int x,i;
  int ch;

  if (Base==10 && ((long int)num<0)) {
    num = -num;
    buffer[0]='-';
    p++;
    }

  x = 0;

  /* generate digits */
  do {
    ch = num % Base;
    num /= Base;

    ch+='0';
    if (ch>'9')  ch+=7;
    p[x++] = ch;
  } while (num);

  /* reverse them */
  for (i=0; i<(x/2); i++) {
    ch = p[i];
    p[i] = p[x-i-1];
    p[x-i-1] = ch;
    }

  p[x] = 0;
  return buffer;
}

void ShowLog(void)
{
  (DrawText)(0,STACK_BOTTOM+10,Log,DRAW_NORM,FONT_SMALL);
}


void ShowBase(void)
{
  char far *basemsg = "BASE\0";

  /*(DrawText)(0,STACK_TOP/ *BOTTOM* /, basemsg,DRAW_NORM,FONT_SMALL);*/

  /* TODO: need to cast depth as unsigned long? */
  (DrawText)(0,STACK_BOTTOM,FormatNum(depth),DRAW_NORM,FONT_SMALL);

}


void Cursor(int on)
{
 if (on) {
   _asm {
     mov ax,0dc00h       ; define cursor data
     mov si,offset cursordata
     int 10h

     mov ax,0dc03h       ; move cursor
     mov cx,STACK_LEFT	 ; 640-16
     mov dx,STACK_TOP/*BOTTOM*/
     int 10h

     mov ax,0dc04h       ; blink cursor
     int 10h

     mov ax,0dc02h       ; blink rate
     mov cx,9
     int 10h

     mov ax,0dc06h       ; turn on cursor
     int 10h
     }
   }
 else {
   _asm {
     mov ax,0dc07h       ; turn off graphics cursor
     int 10h
     }
   }
}


void PushStack(/*long int*/ char num)
{
  if (depth<STACK_DEPTH)  Stack[depth++] = num;
}

/*long int*/
char PopStack(int *err)
{
  if (!depth) *err=1;
         else return Stack[--depth];
}

int ShowEntry(char far *ptr, int y)
{
  char buffer[33];
  int i,len,len2;
  char ch;

  /* Get length of string */
  for (len=0; ptr[len]; len++)  ;
  len2 = len;

  /* Fill up buffer from back to front */
  for (i=sizeof(buffer)-2; i>=0; i--) {
    if (len) ch = ptr[--len];
        else ch = ' ';
    buffer[i] = ch;
    }

  /* terminate string */
  buffer[sizeof(buffer)-1] = 0;

  (DrawText)(STACK_LEFT+(len2*CHAR_WIDTH(FONT_SMALL)),y,buffer,DRAW_NORM,FONT_SMALL);

  /* (DrawText)(STACK_LEFT,y,buffer,DRAW_NORM,FONT_SMALL); */
}


int ShowChar(char c, int y)
{
  char buffer[33];
  int i,len,len2;
  char ch;

  len2 = 1;

  /* Get length of string */
  /*
  for (len=0; ptr[len]; len++)  ;
  len2 = len;
  */

  /* Fill up buffer from back to front */
  /*
  for (i=sizeof(buffer)-2; i>=0; i--) {
    if (len) ch = ptr[--len];
        else ch = ' ';
    buffer[i] = ch;
    }
  */
  /* terminate string */
  /*
  buffer[sizeof(buffer)-1] = 0;
  */

  buffer[0] = c;
  buffer[1] = 0;

  (DrawText)(STACK_LEFT+(len2*CHAR_WIDTH(FONT_SMALL)),y,buffer,DRAW_NORM,FONT_SMALL);

  /* (DrawText)(STACK_LEFT,y,buffer,DRAW_NORM,FONT_SMALL); */
}


int ShowAll()
{

  /*
  #define STACK_DEPTH 80
  #define STACK_HEIGHT 40
  */

  char buffer[STACK_DEPTH+1];
  int i,len;
  char ch;

  strcpy(Log, "first row");
  for (i=0; i<80; i++) {
    if (i < depth) {
      ch = Stack[i];
    } else {
      ch = ' ';
    }
    buffer[i] = ch;
  }

  /* terminate string */
  buffer[STACK_DEPTH] = 0;

  (DrawText)(STACK_LEFT,STACK_TOP,buffer,DRAW_NORM,FONT_SMALL);

  strcpy(Log, "second row");
  if (depth > STACK_DEPTH) {
    for (i=STACK_DEPTH; i<STACK_DEPTH*2; i++) {
      if (i < depth) {
	ch = Stack[i];
      } else {
	ch = ' ';
      }
      buffer[i] = ch;
    }

    /* terminate string */
    buffer[80] = 0;

    (DrawText)(STACK_LEFT,STACK_TOP+10,buffer,DRAW_NORM,FONT_SMALL);

  }

  /* terminate string */
  /* Stack[depth] = 0; */
  /*
  (DrawText)(STACK_LEFT+(len2*CHAR_WIDTH(FONT_SMALL)),y,buffer,DRAW_NORM,FONT_SMALL);
  */
  /* (DrawText)(STACK_LEFT,y,buffer,DRAW_NORM,FONT_SMALL); */

  /*(DrawText)(STACK_LEFT,STACK_TOP,Stack,DRAW_NORM,FONT_SMALL);*/

  strcpy(Log, "done");
}

char *FormatX(unsigned long int num)
{
  static char buffer[33];
  char *p=buffer;
  int x,i;
  int ch;

  x = 0;

  /* generate digits */
  do {
    ch = num % Base;
    num /= Base;

    ch+='0';
    if (ch>'9')  ch+=7;
    p[x++] = ch;
  } while (num);

  /* reverse them */
  for (i=0; i<(x/2); i++) {
    ch = p[i];
    p[i] = p[x-i-1];
    p[x-i-1] = ch;
    }

  p[x] = 0;
  return buffer;
}



void Redisplay(int deep)
{
  int i,y=STACK_TOP;

  if (startnumber || (depth && !Stack[depth-1])) Cursor(0);

  y = STACK_TOP; /*BOTTOM;*/
  for (i=0; i<=deep; i++) {
    if (i<depth)
      /* ShowEntry(FormatNum(Stack[depth-i-1]),y); */
      /* ShowEntry(FormatX(Stack[depth-i-1]),y); */
      /* ShowChar(Stack[depth-i-1],y); */
      ShowAll();

    else
      ShowEntry(msgNull,y);
    y /*-*/+= 16;

    if (y>/*<*/STACK_BOTTOM/*TOP*/)	return;
    }
}


int far MyCardHandler(PWINDOW Wnd, WORD Message, WORD Data, WORD Extra)
{
  switch (Message) {
    case KEYSTROKE:

      if (Data>='a' && Data<='z') {
	PushStack(Data);
      } else {
	switch (Data) {
	  case 8:
	    depth--;
	    break;
	  case 13:
	    depth+=40;
	    break;
	}
      }
      if (depth == 0) {
	depth = 1;
      }

    case DRAW:
       if (Data&DRAW_FRAME) {
          ClearRect(Wnd->x,Wnd->y,Wnd->w,Wnd->h);
          }
       if (Data&DRAW_TITLE) {
	 Rectangle(Wnd->x, Wnd->y, Wnd->w, TITLE_HEIGHT, 1, G_SOLIDFILL);

         (DrawText)(Wnd->x+(Wnd->w>>1)-lstrlen(*(Wnd->Title))*(CHAR_WIDTH(FONT_NORM)/2),
		    Wnd->y+1, *(Wnd->Title), DRAW_INVERT, FONT_SMALL);
	 /*
	 (DrawText)(Wnd->x-(Wnd->w>>1)+lstrlen(*(Wnd->Title))*(CHAR_WIDTH(FONT_NORM)/2),
		    Wnd->y+1, *(Wnd->Title), DRAW_INVERT, FONT_SMALL);
	 */
		 }
       Redisplay(depth);
       ShowBase();
       ShowLog();
       break;

    }

  SubclassMsg(Object, Wnd, Message, Data, Extra);
}




int ProcessEvent(EVENT *app_event)
{
    switch (app_event->kind) {      /* Branch on SysMgr event */
      case E_REFRESH:
      case E_ACTIV:
         FixupFarPtrs();
         ReactivateCAP(&CapData);
         if (!startnumber && (depth && Stack[depth]))  Cursor(1);
         break;

      case E_DEACT:
         Cursor(0);
         DeactivateCAP();
         break;

      case E_TERM:
         FixupFarPtrs();
         Done = TRUE;
         break;

      case E_KEY:
         /* Now send key off to current focus (KeyCode converts gray 101-key */
         /*   arrows/movement scan codes into "normal" scan codes) */
         SendMsg(GetFocus(), KEYSTROKE,
                 Fix101Key(app_event->data,app_event->scan),
                 app_event->scan);   /* Make sure we send the scan code too */
         break;
      }
}



void EventDispatcher(void)
/***
 ***  EventDispatcher grabs events from the System Manager and translates
 ***  them into CAP messages.  Every program will have an Event Dispatcher,
 ***  and the structure should follow this one.
 ***/
{
  Done = FALSE;                    /* Set terminate flag to FALSE */

  while (!Done) {                  /* While loop not terminated */
    app_event.do_event = DO_EVENT;

    m_action(&app_event);           /* Grab system manager event */
    ProcessEvent(&app_event);
    }

}


void LoadENV(void)
{
  int handle,e;

  startnumber=1;

  if (_dos_open(msgENVName,0,&handle)==-1)  goto BadEnv;
  _dos_read(handle,Stack,sizeof(Stack),&e);
  _dos_read(handle,&depth,sizeof(depth),&e);
  _dos_read(handle,&Base,sizeof(Base),&e);
  _dos_close(handle);

  /* Couldn't do it properly */
  if (e!=sizeof(Base)) {
BadEnv:
    depth=0;
    PushStack(0);      /* Start off showing something (if very first time)*/
    startnumber=0;
    }
}


void SaveENV(void)
{
  int handle,e;
  if (_dos_creat(msgENVName,0,&handle)==-1) return;
  _dos_write(handle,Stack,sizeof(Stack),&e);
  _dos_write(handle,&depth,sizeof(depth),&e);
  _dos_write(handle,&Base,sizeof(Base),&e);
  _dos_close(handle);
}



void Uninitialize(void)
/***
 *** Deinstall CAP and SysMgr
 ***/
{
  Cursor(0);
  SaveENV();
  m_fini();
}




void Initialize(void)
/***
 *** Initialize CAP, SysMgr and data structures
 ***/
{
  m_init_app(SYSTEM_MANAGER_VERSION);
  InitializeCAP(&CapData);
//  m_reg_far(&SysMgrFarPtrs, countof(SysMgrFarPtrs), 0);

  SetMenuFont(FONT_NORMAL);
  SetFont(FONT_NORMAL);

//  InitResourcedMessages();

  m_reg_app_name(msgTestApp);

  /* TODO: restore */
  /*LoadENV();*/
  CreateMainView();  /* Now create the index view (it will display itself) */
  EnableClock(TRUE);
}







void main(void)
/***
 *** C main code
 ***/
{
#ifdef TKERNEL
  CheckTSRs();
#endif
  Initialize();           /* Get started */
  EventDispatcher();      /* Do stuff */
  Uninitialize();         /* Get outta there */
}
