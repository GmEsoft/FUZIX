#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>

struct popen_child {
   FILE *stream;
   pid_t pid;
   struct popen_child *next;
};

static struct popen_child *popen_children;

FILE *popen(const char *command, const char *rw)
{
   int pipe_fd[2];
   int reading, saved_errno;
   pid_t pid;
   FILE *fp;
   struct popen_child *child;

   if (strcmp(rw, "r") && strcmp(rw, "w")) {
      errno = EINVAL;
      return NULL;
   }
   child = malloc(sizeof(*child));
   if (child == NULL)
      return NULL;
   if (pipe(pipe_fd) < 0) {
      free(child);
      return NULL;
   }
   reading = rw[0] == 'r';
   /* Allocate the stream before fork so failure cannot leave a child behind. */
   fp = fdopen(pipe_fd[!reading], rw);
   if (fp == NULL) {
      saved_errno = errno;
      close(pipe_fd[0]);
      close(pipe_fd[1]);
      free(child);
      errno = saved_errno;
      return NULL;
   }
   pid = fork();
   if (pid < 0) {
      saved_errno = errno;
      fclose(fp);
      close(pipe_fd[reading]);
      free(child);
      errno = saved_errno;
      return NULL;
   }
   if (pid == 0) {
      struct popen_child *p;
      close(pipe_fd[!reading]);
      /* Do not retain other popen pipes across exec. */
      for (p = popen_children; p; p = p->next)
         close(fileno(p->stream));
      if (pipe_fd[reading] != reading) {
         if (dup2(pipe_fd[reading], reading) < 0)
            _exit(127);
         close(pipe_fd[reading]);
      }
      execl("/bin/sh", "sh", "-c", command, (char *)0);
      _exit(127);
   }
   close(pipe_fd[reading]);
   child->stream = fp;
   child->pid = pid;
   child->next = popen_children;
   popen_children = child;
   return fp;
}

int pclose(FILE *fp)
{
   struct popen_child **link = &popen_children;
   struct popen_child *child;
   pid_t pid, result;
   int status, close_error, saved_errno, fd;

   while (*link && (*link)->stream != fp)
      link = &(*link)->next;
   if (*link == NULL) {
      errno = EINVAL;
      return -1;
   }
   child = *link;
   pid = child->pid;
   *link = child->next;
   free(child);
   fd = fileno(fp);
   close_error = fclose(fp);
   saved_errno = errno;
   /* FUZIX fclose can leave the descriptor open after a flush error. */
   if (close_error)
      close(fd);
   do {
      result = waitpid(pid, &status, 0);
   } while (result == -1 && errno == EINTR);
   if (close_error) {
      errno = saved_errno;
      return -1;
   }
   if (result == -1)
      return -1;
   return status;
}
