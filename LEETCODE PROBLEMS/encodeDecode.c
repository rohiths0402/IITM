#define MAX_URLS 1000
char* longUrls[MAX_URLS];
char* shortUrls[MAX_URLS];
int urlCount = 0;

/** Encodes a URL to a shortened URL. */
char* encode(char* longUrl){
    for(int i = 0; i < urlCount; i++){
        if(strcmp(longUrls[i], longUrl) == 0){
            return shortUrls[i];
        }
    }
    longUrls[urlCount] = strdup(longUrl);
    char buffer[100];
    sprintf(buffer, "http://tinyurl.com/%d", urlCount);
    shortUrls[urlCount] = strdup(buffer);
    urlCount++;
    return shortUrls[urlCount - 1];
}

char* decode(char* shortUrl){
    for(int i = 0; i < urlCount; i++){
        if(strcmp(shortUrls[i], shortUrl) == 0){
            return longUrls[i];
        }
    }
    return NULL;
}