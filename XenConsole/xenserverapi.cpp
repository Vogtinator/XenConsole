#include "xenserverapi.h"

//#define PRINT_XML

typedef struct
{
    xen_result_func func;
    void *handle;
} xen_comms;

static size_t write_func(void *ptr, size_t size, size_t nmemb, xen_comms *comms)
{
    size_t n = size * nmemb;
#ifdef PRINT_XML
    printf("\n\n---Result from server -----------------------\n");
    printf("%s\n",((char*) ptr));
    fflush(stdout);
#endif
    return comms->func(ptr, n, comms->handle) ? n : 0;
}

int call_func(const void *data, size_t len, void *user_handle, void *result_handle, xen_result_func result_func)
{
    #ifdef PRINT_XML
        printf("\n\n---Data to server: -----------------------\n");
        printf("%s\n",((char*) data));
        fflush(stdout);
    #endif

    CURL *curl = curl_easy_init();
    if (!curl) {
        return -1;
    }

    xen_comms comms = {
        .func = result_func,
        .handle = result_handle
    };

    curl_easy_setopt(curl, CURLOPT_URL, reinterpret_cast<xshost_data*>(user_handle)->url.toUtf8().data());
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 1500);
    #ifdef CURLOPT_MUTE
        curl_easy_setopt(curl, CURLOPT_MUTE, 1);
    #endif
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &write_func);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &comms);
    curl_easy_setopt(curl, CURLOPT_POST, 1);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, len);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);

    CURLcode result = curl_easy_perform(curl);

    curl_easy_cleanup(curl);

    return result;
}
