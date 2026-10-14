#include <time.h>
#include <unistd.h>
#include <errno.h>

/* Ok so this doesn't score too highly on accuracy but it'll wrap code using
   the modern API's for long sleeps just fine */

int clock_nanosleep(clockid_t clock, int flags, const struct timespec *reqp, struct timespec *rem)
{
  struct timespec tbase, tend, req;
  uint16_t t;

  if (reqp->tv_sec < 0 || reqp->tv_nsec < 0 ||
      reqp->tv_nsec >= 1000000000L || (flags & ~TIMER_ABSTIME)) {
    errno = EINVAL;
    return -1;
  }

  if (clock_gettime(clock, &tbase))
   return -1;
 
  req.tv_sec = reqp->tv_sec;
  req.tv_nsec = reqp->tv_nsec;

  /* Absolute time means we are handed a time relative to system 0 not
     a time relative to 'now' */
  if (flags & TIMER_ABSTIME) {
    if (req.tv_sec < tbase.tv_sec ||
        (req.tv_sec == tbase.tv_sec && req.tv_nsec <= tbase.tv_nsec))
      return 0;
    req.tv_sec -= tbase.tv_sec;
    req.tv_nsec -= tbase.tv_nsec;
    if (req.tv_nsec < 0) {
     req.tv_nsec += 1000000000L;
     --req.tv_sec;
    }
    if (req.tv_sec < 0)
     return 0;
    /* req is now the relative time for all cases */
  }
  /* Keep every kernel timeout below the 16-bit wrap point. */
  while (req.tv_sec >= 6553) {
    if (_pause(65530))
      goto interrupted;
    req.tv_sec -= 6553;
  }
  t = (uint16_t)req.tv_sec * 10;
  /* Round UP to the kernel's decisecond resolution. */
  while (req.tv_nsec > 0) {
    req.tv_nsec -= 100000000L;
    t++;
  }
  if (t == 0 || _pause(t) == 0)
    return 0;
interrupted:
  /* When did we finish */
  clock_gettime(clock, &tend);

  /* Remaining time is the request minus elapsed time, not elapsed time. */
  if (!(flags & TIMER_ABSTIME) && rem) {
    if (tend.tv_sec < tbase.tv_sec ||
        (tend.tv_sec == tbase.tv_sec && tend.tv_nsec < tbase.tv_nsec)) {
      tend.tv_sec = 0;
      tend.tv_nsec = 0;
    } else {
      tend.tv_sec -= tbase.tv_sec;
      tend.tv_nsec -= tbase.tv_nsec;
      if (tend.tv_nsec < 0) {
        tend.tv_sec--;
        tend.tv_nsec += 1000000000L;
      }
    }
    if (tend.tv_sec > reqp->tv_sec ||
        (tend.tv_sec == reqp->tv_sec && tend.tv_nsec >= reqp->tv_nsec)) {
      rem->tv_sec = 0;
      rem->tv_nsec = 0;
    } else {
      rem->tv_sec = reqp->tv_sec - tend.tv_sec;
      rem->tv_nsec = reqp->tv_nsec - tend.tv_nsec;
      if (rem->tv_nsec < 0) {
        rem->tv_sec--;
        rem->tv_nsec += 1000000000L;
      }
    }
  }
  /* Absolute sleeps leave rem untouched. */
  return -1;
}


int nanosleep(const struct timespec *req, struct timespec *rem)
{
 return clock_nanosleep(CLOCK_REALTIME, 0, req, rem);
}

