--- libclamav/regex_pcre.c~	2017-11-28 14:40:56.484208243 +0100
+++ libclamav/regex_pcre.c	2017-11-28 14:41:07.301207800 +0100
@@ -112,7 +112,8 @@ int cli_pcre_addoptions(struct cli_pcre_
 #if USING_PCRE2
 int cli_pcre_compile(struct cli_pcre_data *pd, long long unsigned match_limit, long long unsigned match_limit_recursion, unsigned int options, int opt_override)
 {
-    int errornum, erroffset;
+    int errornum;
+    size_t erroffset;
     pcre2_general_context *gctx;
     pcre2_compile_context *cctx;
 
