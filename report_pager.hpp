/* Cannonball for x68 - X68000 port by Mattsuan-Z@ColtMaz.
   Part of a derivative work of Cannonball (Copyright Chris White and the Cannonball team),
   released under the Cannonball licence (LICENSE / LICENSE.txt). */
// r028: the figures printed at the end are longer than one screen. On the X68000 they stop when the
// screen is full and wait for a key ("-- HIT ANY KEY --"). On the PC they are printed as before.
#pragma once
#include <cstdio>
#include <cstdarg>
#include <cstring>
#ifdef OR_X68K
#include <x68k/iocs.h>
#endif
inline unsigned& report_pager_rows(){static unsigned r=0;return r;}
inline bool& report_pager_active(){static bool a=false;return a;}
inline void report_pager_start(){report_pager_rows()=0;report_pager_active()=true;}
inline void report_pager_text(const char*s){
#ifdef OR_X68K
 if(report_pager_active()){
  const unsigned COLS=96,PAGE=27;   // Human68k console 96x31; keep room for the prompt line
  while(*s){
   const char*e=std::strchr(s,'\n');size_t n=e?size_t(e-s):std::strlen(s);
   unsigned rows=n?unsigned((n+COLS-1)/COLS):1;
   if(report_pager_rows()+rows>PAGE){
    std::fputs("-- HIT ANY KEY --",stdout);std::fflush(stdout);
    while(_iocs_b_keysns())_iocs_b_keyinp();   // keys left over from the game
    _iocs_b_keyinp();
    std::fputs("\r                 \r",stdout);report_pager_rows()=0;
   }
   std::fwrite(s,1,n,stdout);if(e)std::fputc('\n',stdout);
   if(e)report_pager_rows()+=rows;
   s+=n+(e?1:0);
  }
  std::fflush(stdout);return;
 }
#endif
 std::fputs(s,stdout);
}
inline int report_printf(const char*fmt,...){
 char b[1024];va_list a;va_start(a,fmt);int n=std::vsnprintf(b,sizeof b,fmt,a);va_end(a);
 report_pager_text(b);return n;
}
inline int report_puts(const char*s){report_pager_text(s);report_pager_text("\n");return 1;}
