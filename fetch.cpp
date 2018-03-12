#include <iostream>
#include <string>
#define CURL_STATICLIB
#include <curl/curl.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#ifdef USEGTK
#include <gtk/gtk.h>
#endif

using namespace std;

size_t write_cb(void *, size_t , size_t , void *);
class HttpDownload;

class HttpDownload {
	public:
		HttpDownload(string url) : m_url(url) {}
		friend size_t write_cb(void *, size_t, size_t, void *);

		int download() {
			CURL *curl_handle;
			CURLcode curl_res;
			long response = 0;

			curl_global_init(CURL_GLOBAL_ALL);

			curl_handle = curl_easy_init();
			curl_easy_setopt(curl_handle, CURLOPT_URL, m_url.c_str());
			curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_cb);
			curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void*)this);
			curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1L);

			curl_res = curl_easy_perform(curl_handle);

			if (curl_res == CURLE_OK) {
				curl_easy_getinfo(curl_handle, CURLINFO_RESPONSE_CODE, &response);
				response = (response == 200);
			}

			curl_easy_cleanup(curl_handle);

			curl_global_cleanup();

			return response;
		}

		string contents() { return m_contents; }
	private:
		void append(string to_append) {
			m_contents += to_append;
		}
			
		string m_url;
		string m_contents;
};

size_t write_cb(void *data, size_t size, size_t items, void *downloader) {
	HttpDownload *dl = (HttpDownload*)downloader;
	char *contents = (char*)calloc(items+1, size);
	memcpy(contents, data, size * items);
	
	dl->append(string(contents));

	free(contents);
	return size*items;
};

void usage(char *basename) {
	cout << "Usage: " << endl;
	cout << basename << " ";
#ifdef USEGTK
	cout << "[-c]" << endl;
	cout << "-c:\tPut IP on clipboard.";
#endif
	cout << endl;
}

int main(int argc, char **argv) {
	bool fetch_success = false;	
#ifdef USEGTK
	GtkClipboard *clipboard = NULL;
	const gchar *clipboard_text = NULL;
	bool use_clipboard = false;
#endif

	if (argc == 2) {
#ifdef USEGTK
		if (!strcmp(argv[1], "-c"))
			use_clipboard = true;
#else
		if (0) {}
#endif
		else {
			usage(argv[0]);
			return 1;
		}
	} else if (argc > 2) {
		usage(argv[0]);
		return 1;
	}
#ifdef USEGTK
	gtk_init(&argc, &argv);
#endif

	HttpDownload dl("http://api.ipify.org");

	if (dl.download())
		fetch_success = true;
	
	if (fetch_success)
		cout << dl.contents() << endl;

#ifdef USEGTK
	clipboard_text = dl.contents().c_str();
	clipboard = gtk_clipboard_get(GDK_NONE);
	gtk_clipboard_set_text(clipboard, clipboard_text, -1);

	if (fetch_success && use_clipboard) {
		cout << "Press Ctrl-C after you paste the address from the clipboard...\n";
		gtk_main();
	}
#endif

	return !fetch_success;
}
