#include <stdio.h>
#include <curl/curl.h>
#include <zip.h>
#include <string>
#include "../../cmsis.h"

using namespace std;
class WgetBase {
  CURL  * curl;
  public:
    explicit WgetBase () : curl (nullptr) {
      curl_global_init (CURL_GLOBAL_DEFAULT);
    }
    bool set_url (const string url) {
      curl = curl_easy_init();
      if (!curl) return false;
      curl_easy_setopt (curl, CURLOPT_URL, url.c_str());
      curl_easy_setopt (curl, CURLOPT_WRITEFUNCTION, write_cb);
      curl_easy_setopt (curl, CURLOPT_WRITEDATA,     this);
      curl_easy_setopt (curl, CURLOPT_FOLLOWLOCATION, 1L);
      curl_easy_setopt (curl, CURLOPT_USERAGENT, "Mozilla/5.0 (Windows NT 5.1; rv:52.0)");
      // curl_easy_setopt (curl, CURLOPT_USERAGENT, "curl/7.68.0"); // tohle nefunguje
      // curl_easy_setopt (curl, CURLOPT_VERBOSE, 1L);
      return true;
    }
    bool run () {
      /* Perform the request, res will get the return code */
      CURLcode res = curl_easy_perform (curl);
      /* Check for errors */
      if (res != CURLE_OK) {
        fprintf (stderr, "curl_easy_perform() failed: %s\n",  curl_easy_strerror (res));
        return false;
      }
      return true;
    }
    virtual size_t write_fc (void * buffer, size_t size, size_t nmemb) = 0;
    virtual ~WgetBase () {
      if (curl) curl_easy_cleanup (curl);
      curl_global_cleanup();
    }
    static size_t write_cb (void * buffer, size_t size, size_t nmemb, void * hsp) {
      WgetBase * cb = reinterpret_cast<WgetBase*> (hsp);
      return cb->write_fc(buffer, size, nmemb);
    }
};
class Wget : public WgetBase {
  FILE * out;
  public:
    explicit Wget (const char * localfilename) : WgetBase (), out(nullptr) {
      out = fopen (localfilename,"w");
    };
    ~Wget () override {
      if (out) fclose (out);
    };
    size_t write_fc (void * buffer, size_t size, size_t nmemb) override {
      if (!out) return 0;
      printf ("chunk = %zd\r", nmemb);
      fflush (stdout);
      return fwrite (buffer, size, nmemb, out);
    }
};
static void patch_zero_len (string & contents) {
  const string orig ("<bitWidth>0</bitWidth>");
  const string repl ("<bitWidth>1</bitWidth>");
  const size_t n = contents.find (orig);
  if (n == string::npos) return;
  contents.replace (n, orig.size(), repl);
}
static bool unzip_local (const char * filename) {
  string path ("STM32F0_svd_V1.3/STM32F0x1.svd");
  const size_t e = path.find_last_of ("/");
  const char * svd_file = path.c_str ();
  const char * out_file = path.substr (e + 1).c_str();
  int err = 0;
  zip * z = zip_open(filename, 0, & err);
  struct zip_stat st;
  zip_stat_init(&st);
  zip_stat(z, svd_file, 0, &st);
  zip_file * f = zip_fopen(z, svd_file, 0);
  if (!f) return false;
  char * contents = new char [st.size];
  zip_fread(f, contents, st.size);
  zip_fclose(f);
  zip_close(z);
  string orig (contents, st.size);
  delete[] contents;
  patch_zero_len (orig);
  FILE * out = fopen (out_file, "w");
  if (!out) return false;
  const size_t l = fwrite (orig.c_str(), 1, orig.size(), out);
  if (l != st.size) return false;
  fclose(out);
  remove(filename);
  create_from_svd (out_file, DECLARE_ONLY);
  remove(out_file);
  return true;
}
static bool get_local (const char * filename) {
  Wget w (filename);
  string url ("https://www.st.com/resource/en/svd/stm32f0_svd.zip");
  if (!w.set_url(url)) return false;
  if (!w.run())        return false;
  return true;
}
int main (int argc, char * argv []) {
  const char * local = "proto_svd.zip";
  if (!get_local  (local)) return 1;
  if (!unzip_local(local)) return 2;
  return 0;
}

