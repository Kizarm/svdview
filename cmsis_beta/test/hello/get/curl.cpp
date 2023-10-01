#include <stdio.h>
#include <curl/curl.h>
#include <string>

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
int main (int argc, char * argv []) {
  if (argc < 2) {
    printf("Usage:\n\t%s url\n", argv [0]);
    return -1;
  }
  string url (argv [1]);
  size_t slash = url.find_last_of ('/');
  string local = url.substr (slash + 1);
  if (local.empty()) {
    fprintf(stderr, "local filename empty\n");
    return 1;
  }
  Wget w (local.c_str());
  if (!w.set_url(url)) return 2;
  if (!w.run())        return 3;
  return 0;
}

