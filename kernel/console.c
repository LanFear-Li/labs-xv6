//
// Console input and output, to the uart.
// Reads are line at a time.
// Implements special input characters:
//   newline -- end of line
//   control-h -- backspace
//   control-u -- kill line
//   control-d -- end of file
//   control-p -- print process list
//

#include <stdarg.h>

#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "fs.h"
#include "file.h"
#include "memlayout.h"
#include "riscv.h"
#include "defs.h"
#include "proc.h"
#include "signal.h"

#define CONSOLE_N   2

#define BACKSPACE 0x100
#define C(x)  ((x)-'@')  // Control-x

//
// send one character to the uart.
// called by printf, and to echo input characters,
// but not from write().
//
void
consputc(int i, int c)
{
  if(c == BACKSPACE){
    // if the user typed backspace, overwrite with a space.
    uartputc_sync(i, '\b'); uartputc_sync(i, ' '); uartputc_sync(i, '\b');
  } else {
    uartputc_sync(i, c);
  }
}

struct cons {
  struct spinlock lock;
  
  // input
#define INPUT_BUF 128
  char buf[INPUT_BUF];
  uint r;  // Read index
  uint w;  // Write index
  uint e;  // Edit index
} cons[CONSOLE_N];

//
// user write()s to the console go here.
//
int
consolewrite(int i, int user_src, uint64 src, int n)
{
  int k;

  if (i >= CONSOLE_N) {
    return -1;
  }

  for(k = 0; k < n; k++){
    char c;
    if(either_copyin(&c, user_src, src + k, 1) == -1)
      break;
    uartputc(i, c);
  }

  return k;
}

//
// user read()s from the console go here.
// copy (up to) a whole input line to dst.
// user_dist indicates whether dst is a user
// or kernel address.
//
int
consoleread(int i, int user_dst, uint64 dst, int n)
{
  struct cons *cs = &cons[i];
  uint target;
  int c;
  char cbuf;

  if (i >= CONSOLE_N) {
    return -1;
  }

  target = n;
  acquire(&cs->lock);
  while(n > 0){
    // wait until interrupt handler has put some
    // input into cs->buffer.
    while(cs->r == cs->w){
      if(myproc()->killed || signal_pending()){
        release(&cs->lock);
        return -1;
      }
      sleep(&cs->r, &cs->lock);
    }

    c = cs->buf[cs->r++ % INPUT_BUF];

    if(c == C('D')){  // end-of-file
      if(n < target){
        // Save ^D for next time, to make sure
        // caller gets a 0-byte result.
        cs->r--;
      }
      break;
    }

    // copy the input byte to the user-space buffer.
    cbuf = c;
    if(either_copyout(user_dst, dst, &cbuf, 1) == -1)
      break;

    dst++;
    --n;

    if(c == '\n'){
      // a whole line has arrived, return to
      // the user-level read().
      break;
    }
  }
  release(&cs->lock);

  return target - n;
}

//
// the console input interrupt handler.
// uartintr() calls this for input character.
// do erase/kill processing, append to cons.buf,
// wake up consoleread() if a whole line has arrived.
//
void
consoleintr(int i, int c)
{
  struct cons *cs = &cons[i];
    
  acquire(&cs->lock);

  switch(c){
  case C('C'):
    ttyintr(i, 'C');
    break;
  case C('Z'):
    ttyintr(i, 'Z');
    break;
  case C('P'):  // Print process list.
    procdump();
    break;
  case C('U'):  // Kill line.
    while(cs->e != cs->w &&
          cs->buf[(cs->e-1) % INPUT_BUF] != '\n'){
      cs->e--;
      consputc(i, BACKSPACE);
    }
    break;
  case C('H'): // Backspace
  case '\x7f':
    if(cs->e != cs->w){
      cs->e--;
      consputc(i, BACKSPACE);
    }
    break;
  default:
    if(c != 0 && cs->e-cs->r < INPUT_BUF){
      c = (c == '\r') ? '\n' : c;

      // echo back to the user.
      consputc(i, c);

      // store for consumption by consoleread().
      cs->buf[cs->e++ % INPUT_BUF] = c;

      if(c == '\n' || c == C('D') || cs->e == cs->r+INPUT_BUF){
        // wake up consoleread() if a whole line (or end-of-file)
        // has arrived.
        cs->w = cs->e;
        wakeup(&cs->r);
      }
    }
    break;
  }
  
  release(&cs->lock);
}

void
consoleinit()
{
  int i;

  for (i = 0; i < CONSOLE_N; i++) {
    initlock(&cons[i].lock, "cons");
    uartinit(i);
  }

  // connect read and write system calls
  // to consoleread and consolewrite.
  devsw[CONSOLE].read = consoleread;
  devsw[CONSOLE].write = consolewrite;
}
