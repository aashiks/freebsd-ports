--- coolmail.c.orig	1996-01-11 18:57:24.000000000 +0100
+++ coolmail.c	2009-04-08 09:14:08.000000000 +0200
@@ -25,8 +25,10 @@
 #include <sys/wait.h>
 #include <sys/types.h>
 #include <sys/stat.h>
+#include <dirent.h>
 #include <fcntl.h>
 
+
 #ifdef AUDIO
 #include <string.h>
 #endif
@@ -34,14 +36,13 @@
 #include "render1.h"
 #include "mailbox.h"
 
-#define DEFAULT_MAIL_DIR  "/var/spool/mail/"
-#define DEFAULT_COMMAND   "xterm -n Elm -e mail\0"
+#define DEFAULT_MAIL_DIR  "/var/mail/"
+
+#define DEFAULT_COMMAND   "xterm -n Elm -e elm\0"
 #define DEFAULT_INTERVAL  30
 #define DEFAULT_FRAMES    15
 
-#ifdef AUDIO
 #define DEFAULT_VOLUME    50
-#endif
 
 #ifndef PI
 #define PI 3.1415926536
@@ -60,9 +61,9 @@
 unsigned int  frames             = DEFAULT_FRAMES;
 int           verbose            = 0;
 
+int          cool_vol            = DEFAULT_VOLUME;
 #ifdef AUDIO
 char         *sndfile            = NULL;   /* default system sound */
-int          cool_vol            = DEFAULT_VOLUME;
 #endif
 
 float  flag_angle = 0.0;
@@ -96,7 +97,9 @@
 int main(int argc, char *argv[])
 {
    int reason;
+#ifndef NO_CUSERID
    char username[L_cuserid];
+#endif
 
    /* Quickly scan for the -h option -- if it is present don't do anything
     * but print out some help and exit. */
@@ -104,7 +107,18 @@
       return(0);
 
    /* Get the username and use it to create a default mailfile name */
-   strcat(mailfile_str, cuserid(username));
+#ifdef SUPPORT_MAILDIR
+	if (getenv("MAILDIR") && strlen(getenv("MAILDIR"))) {
+		strcpy(mailfile_str,getenv("MAILDIR"));
+	} else if (getenv("MAIL") && strlen(getenv("MAIL"))) {
+   	strcpy(mailfile_str,getenv("MAIL"));
+	} else
+#endif
+#ifndef NO_CUSERID
+   	strcat(mailfile_str, cuserid(username));
+#else
+   	strcat(mailfile_str, getlogin());
+#endif
 
    /* Initialize the renderer */
    rend_init(&argc, argv, (float)150.0);
@@ -379,7 +393,6 @@
       {
          verbose++;
       }
-#ifdef AUDIO
       else if (!strcmp(argv[i], "-vol"))
       {
          i++;
@@ -387,6 +400,7 @@
          if      (cool_vol < 0)   cool_vol = 0;
          else if (cool_vol > 100) cool_vol = 100;
       }
+#ifdef AUDIO
       else if (!strcmp(argv[i], "-af"))
       {
          i++;
@@ -432,7 +446,7 @@
    printf("  -e command   Specifies a command (usually in quotes) which\n");
    printf("               is used to invoke your favorite mail-reading\n");
    printf("               program.\n\n");
-   printf("  -f filename  Watch filename, instead of the default mail\n");
+   printf("  -f filename  Watch filename/maildir, instead of the default mail\n");
    printf("               file, %s<username>.\n\n", DEFAULT_MAIL_DIR);
    printf("  -fr n        Number of frames to generate for each animation.\n");
    printf("               Set to an appropriate value for your machine's.\n");
@@ -626,6 +640,39 @@
 
 /* Get file modification time */
 
+
+/* Maildir notes (aqua@sonoma.net, Sun Jan 18 19:42:27 PST 1998):
+ *
+ * The maildir mail-storage standard is a replacement for the traditional
+ * 'mbox' format, intended to remove problems with file contention, locking,
+ * reduce corruption in the case of a program or system crash, etc, etc.
+ * Fairly detailed description of it can be had as part of the Qmail MTA
+ * documentation, http://www.qmail.org/qmail-manual-html/man5/maildir.html.
+ *
+ * The general gist of the maildir approach is that mail is stored, one
+ # message per file, in a subtree of ~/Maildir.  New mail goes in /new,
+ * the "spool" goes in /cur, and /tmp is available to MUAs &c.  Mail is
+ * theoretically supposed to be removed from /new immediately by the
+ * MUA, but I've observed that with mutt 0.88, at least, it isn't if
+ * the mailfile was generated by an import script (e.g. mbox2maildir)
+ * rather than the normal delivery agent, presumably due to naming
+ * differences.
+ *
+ * Checking for new mail mostly entails checking the mtime vs. atime of
+ * every file in /new, and the number of messages in /new; if the latter
+ * increases, new mail was delivered -- if not, but the files' atimes
+ * are all later than their mtimes, the MUA read the /new spool.
+ *
+ * The specifications suggest skipping over every .file, but reading all
+ * the others -- I've extended this to include skipping of all non-regular
+ * files, which seemed to make sense -- define SUPPORT_MAILDIR_STRICTER to
+ * override this behavior.
+ *
+ * This process is more resource-intensive than the old scheme of merely
+ * calling stat() for a single file -- it's an O(n) rather than O(1)
+ * operation.
+ *
+ */
 void cool_get_inboxstatus(char *filename, int *anymail, int *unreadmail,
                           int *newmail)
 {
@@ -633,38 +680,119 @@
    off_t  newsize;
    struct stat st;
    int fd;
+#ifdef SUPPORT_MAILDIR
+   DIR *d;
+   struct dirent *de;
+   char maildir[256],mfn[256];
+#endif
 
-   fd = open (filename, O_RDONLY, 0);
-   if (fd < 0)
-   {
-      *anymail    = 0;
-      *newmail    = 0;
-      *unreadmail = 0;
-      newsize = 0;
-   }
-   else
-   {
-      fstat(fd, &st);
-      close(fd);
-      newsize = st.st_size;
-
-      if (newsize > 0)
-         *anymail = 1;
-      else
-         *anymail = 0;
-
-      if (st.st_mtime >= st.st_atime && newsize > 0)
-         *unreadmail = 1;
-      else
-         *unreadmail = 0;
-
-      if (newsize > oldsize && *unreadmail)
-         *newmail = 1;
-      else
-         *newmail = 0;
-   }
 
-   oldsize = newsize;
+#ifdef SUPPORT_MAILDIR_DEBUG
+   printf("B anymail=%d, newmail=%d, unreadmail=%d, oldsize=%d, newsize=%d\n",
+   	*anymail,*newmail,*unreadmail,oldsize,newsize);
+#endif
+#ifdef SUPPORT_MAILDIR
+   if (stat(filename,&st)==-1) {
+  	   *anymail = *newmail = *unreadmail = 0;
+     	newsize = oldsize = 0;   
+      perror(filename);
+      return;
+   }
+   if (S_ISDIR(st.st_mode)) {
+      /* likely a maildir */
+   	strcpy(maildir,filename);
+	   if (maildir[strlen(maildir)-1]!='/')
+   	  strcat(maildir,"/");
+	   strcat(maildir,"new");
+   	if (stat(maildir,&st)==-1) {
+      	perror(maildir);
+	      printf("%s is not a maildir (missing/inaccessible %s)\n",filename,maildir);
+   	   *anymail = *newmail = *unreadmail = 0;
+      	newsize = oldsize = 0;
+	      return;
+   	}
+	   if (!S_ISDIR(st.st_mode)) {
+   	   printf("%s is not a directory (mode %d)\n",maildir,st.st_mode);
+      	*anymail = *newmail = *unreadmail = 0;
+	      newsize = oldsize = 0;
+   	   return;
+	   }   
+	   d=opendir(maildir);
+	   newsize=0;
+   	*unreadmail = 0;
+	   while ((de=readdir(d))) {
+   	   if (de->d_name[0]=='.') /* dotfiles ignored per the maildir specs */
+      	   continue;
+	      strcpy(mfn,maildir);
+   	   if (mfn[strlen(mfn)-1]!='/')
+      	  strcat(mfn,"/");
+	      strcat(mfn,de->d_name);
+   	   if (stat(mfn,&st)==-1) {
+      	  perror(mfn);
+	        continue;
+   	   }
+#ifndef SUPPORT_MAILDIR_STRICTER
+      	if (S_ISREG(st.st_mode))
+#endif
+         	newsize++;
+	      if (st.st_mtime>=st.st_atime) {
+#ifdef SUPPORT_MAILDIR_DEBUG
+   	     printf("unread: %s mtime = %d, atime = %d\n",de->d_name,st.st_mtime,st.st_atime);
+#endif
+	        *unreadmail = 1;
+   	   }
+	   }
+	   closedir(d);
+	   if (newsize) {
+	     *anymail = 1;
+   	  if (newsize>oldsize && *unreadmail)
+      	 *newmail = 1;
+	     else
+   	    *newmail = 0;
+	   } else {
+   	  *anymail = *newmail = *unreadmail = 0;
+	     newsize = 0;
+	   }
+#ifdef SUPPORT_MAILDIR_DEBUG
+   	printf("A anymail=%d, newmail=%d, unreadmail=%d, oldsize=%d, newsize=%d\n",
+		  	*anymail,*newmail,*unreadmail,oldsize,newsize);
+#endif
+	   oldsize=newsize;
+	} else {
+#endif /* SUPPORT_MAILDIR */
+	   fd = open (filename, O_RDONLY, 0);
+   	if (fd < 0)
+	   {
+   	   *anymail    = 0;
+      	*newmail    = 0;
+	      *unreadmail = 0;
+   	   newsize = 0;
+	   }
+	   else
+	   {
+   	   fstat(fd, &st);
+	      close(fd);
+   	   newsize = st.st_size;
+
+	      if (newsize > 0)
+   	      *anymail = 1;
+      	else
+	         *anymail = 0;
+
+   	   if (st.st_mtime >= st.st_atime && newsize > 0)
+      	   *unreadmail = 1;
+	      else
+   	      *unreadmail = 0;
+
+	      if (newsize > oldsize && *unreadmail)
+   	      *newmail = 1;
+	      else
+   	      *newmail = 0;
+	   }
+#ifdef SUPPORT_MAILDIR
+	}
+#endif
+	oldsize = newsize;	
 }
 
 /*---------------------------------------------------------------------------*/
