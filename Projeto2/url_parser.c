#include "url_parser.h"

void initialize_struct(url_info* url) {
    url->port = 21;
}

//ip_str = ftp://[<user>:<password>@]<host>/<url-path>
int setUpStruct(url_info* url, const char* ip_str) {
    if (strncmp(ip_str, "ftp://", 6)) {
        printf("Invalid URL syntax, must start with ftp://\n");
        return 1;
    }

    char* temp_url = (char*) malloc(strlen(ip_str));
    char* url_path = (char*) malloc(strlen(ip_str));

	strcpy(temp_url, ip_str);
    
    // removing ftp:// from temp_url
	strcpy(temp_url, temp_url + 6); // temp_url = [<user>:<password>@]<host>/<url-path>

    char* url_rest = strchr(temp_url, '@'); //url rest will null if no user was defined or @<host>/<url-path> otherwise

    if (url_rest == NULL) {
        strcpy(url_path, temp_url);  // url_path = <host>/<url-path>

        strcpy(url->user, "anonymous");
        strcpy(url->password, "any");
    }

    else {
        strcpy(url_path, url_rest + 1); // url_path = <host>/<url-path>
        
        strcpy(url->user, get_str_before_char(temp_url, ':'));
        strcpy(temp_url, temp_url + strlen(url->user) + 1); // temp_url = <password>@<host>/<url-path>
        strcpy(url->password, get_str_before_char(temp_url, '@'));
    }

    strcpy(url->host_name, get_str_before_char(url_path, '/'));

    strcpy(url_path, url_path + strlen(url->host_name) + 1); // url_rest = /<url-path>

    get_url_path(url_path, url->url_path, url->filename);

    struct hostent* h;

    if ((h = gethostbyname(url->host_name)) == NULL) {  
        perror("gethostbyname()\n");
        return 1;
    }

    char* ip = inet_ntoa(*( (struct in_addr *) h->h_addr) );
	strcpy(url->ip_address, ip);

    return 0;
}

int get_url_path(const char* ip_str, char* url_path, char* filename) {
    char* path = (char*) malloc(strlen(ip_str));

    char* working_str = (char*) malloc(strlen(ip_str));
    memcpy(working_str, ip_str, strlen(ip_str));
    char* temp_str = (char*) malloc(strlen(ip_str));

	while (strchr(working_str, '/')) {
        temp_str = get_str_before_char(working_str, '/');
        strcpy(working_str, working_str + strlen(temp_str) + 1);

		strcat(path, temp_str);
		strcat(path, "/");
	}
    
	strcpy(url_path, path);
	strcpy(filename, working_str);

	free(path);
    free(temp_str);

    return 0;
}

char* get_str_before_char(const char* str, const char chr) {
	char* temp = (char*) malloc(strlen(str));
	int index = strlen(str) - strlen(strcpy(temp, strchr(str, chr)));

	temp[index] = '\0'; 
	strncpy(temp, str, index);
	return temp;
}

void printf_info(url_info* url) {
    printf("User: %s\n", url->user);
    printf("Password: %s\n", url->password);
    printf("Host: %s\n", url->host_name);
    printf("IP: %s\n", url->ip_address);
    printf("URL Path: %s\n", url->url_path);
    printf("File: %s\n", url->filename);
}