/*
 * Ondrej Vales, xvales03
 * IMP, programovatelny DEC citac
 * original
*/

#include <hidef.h> /* for EnableInterrupts macro */
#include "derivative.h" /* include peripheral declarations */

/* definition of states */
enum flags {
  LAST_NONE,
  LAST_LEFT,
  LAST_RIGHT,
  END_REACHED
};

volatile unsigned char InputVec @0x00000100 = 0x00;     /* DILswitch */
volatile unsigned char L7seg @0x00000101 = 0x3F;        /* left Seg7 */
volatile unsigned char R7seg @0x00000102 = 0x3F;        /* right Seg7 */
volatile unsigned char Seg7 @0x00000103 = 0x00;         /* inner representatoin of L7Seg and R7Seg values (upper 4 bits L7seg, lower 4 bits R7seg) */
volatile unsigned char InnerVec @0x00000104 = 0x00;     /* inner representatoin of DILswitch value */
volatile unsigned char Last @0x00000105 = LAST_NONE;    /* holds state (type of last operation performed) in the main for loop*/
volatile unsigned char Count @0x00000106 = 0x00;        /* counts end cycles */ 
volatile unsigned char Flash @0x00000107 = 0x00;        /* counts end cycles */ 
volatile unsigned int CNT @0x00000108 = 0x00;           /* 16 bit counter value */

/* ------------------------------------------------------------------------- */
/*                              Kod prevzaty z:                              */
/*          Mikroprocesorove a vestavene systemy IMP Studijni opora          */
/* ------------------------------------------------------------------------- */
/* returns Seg7 representation of hexadecimal value val */
unsigned char seven_segment(int val) { 
  switch(val) { 
    case 0: return 0x3F; 
    case 1: return 0x06; 
    case 2: return 0x5B; 
    case 3: return 0x4F; 
    case 4: return 0x66; 
    case 5: return 0x6D; 
    case 6: return 0x7D; 
    case 7: return 0x07; 
    case 8: return 0x7F; 
    case 9: return 0x6F; 
    case 0xA: return 0x77; 
    case 0xB: return 0x7C; 
    case 0xC: return 0x39; 
    case 0xD: return 0x5E;        
    case 0xE: return 0x79; 
    case 0xF: return 0x71; 
  } 
}
/* ------------------------------------------------------------------------- */
/*                           Konec prevzateho kodu                           */
/* ------------------------------------------------------------------------- */ 

/* reset all variables */
void reset() {
  L7seg = 0x3F;
  R7seg = 0x3F;
  Seg7 = 0x00; 
  CNT = 0x0000;
  Count = 0x00;
  Flash = 0x00;
  Last = LAST_NONE;
}

/* sets value of CNT on index idx to val */
void set_cnt(int idx, int val) { 
  switch(idx) { 
    case 3:
      CNT = ((CNT & 0xFFF0) | (val & 0x000F));
      return; 
    case 2:
      CNT = ((CNT & 0xFF0F) | ((val << 4) & 0x00F0));
      return;  
    case 1:
      CNT = ((CNT & 0xF0FF) | ((val << 8) & 0x0F00));
      return;  
    case 0:
      CNT = ((CNT & 0x0FFF) | ((val << 12) & 0xF000));
      return; 
  } 
}
 
/* returns value of CNT on index idx */
unsigned char get_cnt(int idx) { 
  switch(idx) { 
    case 3:       
      return (CNT & 0x000F);
    case 2:
      return ((CNT & 0x00F0) >> 4);  
    case 1:
      return ((CNT & 0x0F00) >> 8);  
    case 0:
      return ((CNT & 0xF000) >> 12); 
  } 
} 
 
/* decrement value of CNT on index idx */
void dec_cnt(int idx) {
  if (get_cnt(idx) == 0){
    set_cnt(idx, 9);
    dec_cnt(idx - 1);  
  }
  else {
    set_cnt(idx, get_cnt(idx) - 1);    
  }
}
 
/* increment value of CNT on index idx */
void inc_cnt(int idx) {
  if (get_cnt(idx) == 9){
    set_cnt(idx, 0);
    inc_cnt(idx - 1);  
  }
  else {
    set_cnt(idx, get_cnt(idx) + 1);    
  }
}  

void update_7Seg_down() {
unsigned char i;
  for (i = 0; i < 4; i++) {                                 /* loop over all indicies to find first nonzero*/
    if (get_cnt(i) != 0 || i == 3) {                                  /* higest index with nonzero value */
      Seg7 = ((i << 4) | (get_cnt(i) & 0x0F));              /* set index and value*/
      L7seg = seven_segment(Seg7 >> 4);                     /* update shown index */
      R7seg = seven_segment(Seg7 & 0x0F);                   /* update shown value */
      return;
    }
  }  
}

void update_7Seg_up() {
unsigned char i;
  for (i = 0; i < 4; i++) {                                 /* loop over all indicies to find first nonnine*/
    if (get_cnt(i) != 9 || i == 3) {                                  /* higest index with nonnine value */
      Seg7 = ((i << 4) | (get_cnt(i) & 0x0F));              /* set index and value*/
      L7seg = seven_segment(Seg7 >> 4);                     /* update shown index */
      R7seg = seven_segment(Seg7 & 0x0F);                   /* update shown value */
      return;
    }
  }  
}

void all_off() {
  L7seg = 0x00;
  R7seg = 0x00;  
}

void all_on() {
  L7seg = seven_segment(8);
  R7seg = seven_segment(8);  
}

interrupt VectorNumber_Vrtc void RTC_interrupt(void) {
  Flash++;
  if (Last == END_REACHED) {                 /* flash both 7Seg to indicate end of count */
    if (Count % 2 == 0) {
      all_off();
    }
    else {
      all_on();
    }
    Count++;      
  } 
  else {    
    if ((InnerVec & 0xE0) == 0xC0) {          /* count down mode */  
      if (CNT == 0x0000){                     /* minval reached */
        Last = END_REACHED;                   /* set state to end */
        Count = 0;
      }
      else {                                  
        dec_cnt(3);                           /* decrement value in CNT */
      }
    }
    else if ((InnerVec & 0xE0) == 0xE0) {     /* count up mode */  
      if (CNT == 0x9999){                     /* maxval reached */
        Last = END_REACHED;                   /* set state to end */
        Count = 0;
      }
      else {      
        inc_cnt(3);                           /* increment value in CNT */
      }
    }
  }
  RTCSC = RTCSC | 0x80;   /* clear interrupt flag for next interrupt */
} 

void main(void) {

  EnableInterrupts; /* enable interrupts */
  /* include your code here */

  RTCMOD = 0x00;         /* set RTC module to interupt once evry 1 second (0x1F)*/
  RTCSC = 0x1D;          /* due to delay in CodeWarrior interupt is set to once every 1/16 seconds */
   
  for(;;) {
    __RESET_WATCHDOG(); /* feeds the dog */
    
    if (Last == END_REACHED) {
      if (Count == 4)
        reset();      
        InputVec = 0x00;
    }
    else {
      InnerVec = InputVec;    /* make sure vector does not change inside loop */
      if (InputVec < 0x80) {  /* reset, reinitialize all variables */
        reset();
        InputVec = InputVec & 0x80;
      }
      else if ((InnerVec & 0xD0) == 0x80) {               /* set index mode */
        if (Last != LAST_LEFT){                           /* current state is different from previous */
          Last = LAST_LEFT;                               /* set current state */
          InnerVec = ((InnerVec & 0xF0) | (Seg7 >> 4));   /* set control vector to values coresponding with current index */
          InputVec = InnerVec;                            /* update DILswitch */ 
        }     
        if ((InnerVec & 0x0F) <= 3){   
          Seg7 = (((InnerVec & 0x0F) << 4) | (get_cnt(Seg7 >> 4) & 0x0F));    /* set index */
          if (Flash % 2 == 0){
            L7seg = 0x00;                                                     /* flash shown index */
          } 
          else {
            L7seg = seven_segment(Seg7 >> 4);                                 /* update shown index */
          }
          R7seg = seven_segment(Seg7 & 0x0F);                                 /* update shown value */
        }             
      }
      else if ((InnerVec & 0xD0) == 0x90) {               /* set value mode */
        if (Last != LAST_RIGHT){                          /* current state is different from previous */
          Last = LAST_RIGHT;                              /* set current state */
          InnerVec = ((InnerVec & 0xF0) | (get_cnt(Seg7 >> 4) & 0x0F));   /* set control vector to values coresponding with current value */
          InputVec = InnerVec;                            /* update DILswitch */
        }  
        if ((InnerVec & 0x0F) <= 9){   
          Seg7 = ((Seg7 & 0xF0) | (InnerVec & 0x0F));     /* set value */
          if (Flash % 2 == 0){
            R7seg = 0x00;                                 /* flash shown value */
          }
          else {
            R7seg = seven_segment(Seg7 & 0x0F);           /* update shown value */
          }
          set_cnt((Seg7 >> 4), (Seg7 & 0x0F));            /* update value in CNT */
          L7seg = seven_segment(Seg7 >> 4);
        }    
      } 
      else if ((InnerVec & 0xE0) == 0xC0){                /* update 7Seg on count down */
        update_7Seg_down();
      }
      else if ((InnerVec & 0xE0) == 0xE0){                /* update 7Seg on count up */
        update_7Seg_up();
      }
      else {
       Last = LAST_NONE;      /* set current state */
      }
    }
  } /* loop forever */
  /* please make sure that you never leave main */
}
