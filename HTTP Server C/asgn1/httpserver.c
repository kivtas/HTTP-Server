#include "bind.h"
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <regex.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <err.h>

char* getStatus(int code) {
    switch(code) {
        case 200: return "OK";
        case 201: return "Created";
        case 400: return "Bad Request";
        case 403: return "Forbidden";
        case 404: return "Not Found";
        case 500: return "Internal Server Error";
        case 501: return "Not Implemented";
    }
    return NULL;
}

char* getMessage(int code) {
    switch(code) {
        case 200: return "OK\n";
        case 201: return "Created\n";
        case 400: return "Bad Request\n";
        case 403: return "Forbidden\n";
        case 404: return "Not Found\n";
        case 500: return "Internal Server Error\n";
        case 501: return "Not Implemented\n";
    }
    return NULL;
}

void httpresponse(int socket, int code, int length, char *response, char *message) {
    sprintf(response, "HTTP/1.1 %d %s\r\nContent-Length: %d\r\n\r\n%s", code, getStatus(code), length, message);
    int sent = send(socket, response, strlen(response), 0);
    if(sent < 0) {
        return;
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        warnx("wrong arguments: ./httpserver port_num\nusage: ./httpserver <port>");
        exit(1);
    }
    //printf("here");
    int listenfd;
    int acceptfd;
    listenfd = create_listen_socket(atoi(argv[1]));

    while(1) {
        char buffer[512];
        struct stat path_stat;
        char response[2048];
        int code;
        int content_length;
        char request[10000];
        char r_type[10];
        char c_l[3000];
        char URI[3000];
        regex_t r;
        regmatch_t pmatch[5];
        size_t nmatch = 5;
        regmatch_t chmatch[2];
        size_t chnmatch = 2;
        //regmatch_t hmatch[4];
        //size_t hhmatch = 4;
        int value;
        int value2;
        //int value3;
        int n;
        int f;
        char *end;

        const char *request_line_regex = "([a-zA-Z]+){1} (/)([ -~])+ (HTTP/1[.]1)";
        const char *content_length_regex = "(Content-Length: [0-9]+)";
        //const char *header_regex = "(\r\n)([ -~]+[:] [ -~]+\r\n)+(\r\n)";
        acceptfd = accept(listenfd, NULL, NULL);
        if (acceptfd < 0) {
            warnx("bind: Address already in use");
            exit(1);
        }
        while((n = recv(acceptfd, &buffer, sizeof(char), 0)) > 0) {
            strcat(request, buffer);
            end = strstr(request, "\r\n\r\n");
            if(end != NULL) {
                break;
            }
            memset(&buffer, 0, sizeof(buffer));
        }
        value = regcomp(&r, request_line_regex, REG_EXTENDED);
        value = regexec(&r, request, nmatch, pmatch, 0);
        if (value == REG_NOMATCH) {
            code = 400;
            httpresponse(acceptfd, code, strlen(getMessage(code)), response, getMessage(code));
        } else {
            // get request_type and URI
            //printf("here1\n");
            const char *end_of_req = strstr(request, " ");
            const char *start_of_URI = strstr(request, "/") + 1;
            const char *end_of_URI = strchr(start_of_URI, ' ');
            char type[end_of_req - request];
            char realURI[end_of_URI - start_of_URI];
            strncpy(type, request, end_of_req - request);
            strncpy(realURI, start_of_URI, end_of_URI - start_of_URI);
            realURI[sizeof(realURI)] = 0;
            type[sizeof(type)] = 0;
            strcpy(r_type, type);
            strcpy(URI, realURI);
            
            stat(URI, &path_stat);
            
            

            //printf("here2\n");

            //get headers
            /*
            const char *start_of_headers = strstr(request, "\r\n");
            const char *end_of_headers = strstr(start_of_headers, "\r\n\r\n");
            char hf[start_of_headers - end_of_headers];
            strncpy(hf, start_of_headers, end_of_headers - start_of_headers);
            hf[sizeof(hf)] = 0;
            value3 = regcomp(&r, header_regex, REG_EXTENDED);
            value3 = regexec(&r, request, hhmatch, hmatch, 0);
            if(value3 == REG_NOMATCH) {
                code = 400;
                httpresponse(acceptfd, code, strlen(getMessage(code)), response, getMessage(code));
                //memset(&hf, 0 ,sizeof(hf));
                continue;
            }
            */
           


            if(strcmp(r_type, "PUT") == 0) {
                // verify it has Content-Length header
                value2 = regcomp(&r, content_length_regex, REG_EXTENDED);
                value2 = regexec(&r, request, chnmatch, chmatch, 0);
                if (value2 == REG_NOMATCH){
                    code = 400;
                    httpresponse(acceptfd, code, strlen(getMessage(code)), response, getMessage(code));
                } else {
                    // get the content length
                    const char *start_of_cl = strstr(request, "Content-Length: ");
                    const char *end_of_cl = strstr(start_of_cl, "\r\n");
                    char cl[end_of_cl - start_of_cl];
                    strncpy(cl, start_of_cl, end_of_cl - start_of_cl);
                    cl[sizeof(cl)] = 0;
                    strcpy(c_l, cl);
                    char *content_length_s = strchr(cl, ' ') + 1;
                    int c_l_s = atoi(content_length_s);
                    content_length = c_l_s;
                    
                    


                    // Do PUT operations
                    //check if is a directory
                    if(S_ISDIR(path_stat.st_mode)) {
                        code = 403;
                        httpresponse(acceptfd, code, strlen(getMessage(code)), response, getMessage(code));
                    //check if file doesnt exist
                    } else if(!S_ISREG(path_stat.st_mode)){
                        code = 201;
                    } else {
                        //f = open(URI, O_WRONLY|O_TRUNC, 0666);
                        code = 200;
                    }
                    f = open(URI, O_RDWR|O_TRUNC|O_CREAT, 0666);
                    if(f < 0) {
                        code = 403;
                        httpresponse(acceptfd, code, strlen(getMessage(code)), response, getMessage(code));
                    }
                    if (code == 200 || code == 201) {
                        for(int i = 0; i < content_length; i++) {
                            n = recv(acceptfd, &buffer, sizeof(char), 0);
                            if(i == content_length) {
                                break;
                            }
                            write(f, buffer, sizeof(char));
                            memset(&buffer, 0, sizeof(buffer));
                        }
                        memset(&response, 0, sizeof(response));
                        httpresponse(acceptfd, code, strlen(getMessage(code)), response, getMessage(code));
                        close(f);
                        continue;
                    }

                }
                
            } else if (strcmp(r_type, "GET") == 0) {
                // check ifdir or if file exists
                if(S_ISDIR(path_stat.st_mode)) {
                    code = 403;
                    httpresponse(acceptfd, code, strlen(getMessage(code)), response, getMessage(code));
                } else if(!S_ISREG(path_stat.st_mode)){
                    code = 404;
                    httpresponse(acceptfd, code, strlen(getMessage(code)), response, getMessage(code));
                } else {
                    f = open(URI, O_RDONLY);
                    if (f < 0) {
                        code = 403;
                        httpresponse(acceptfd, code, strlen(getMessage(code)), response, getMessage(code));
                    } else {
                        char word[512];
                        int bytes;
                        off_t fsize = path_stat.st_size;
                        char mb[fsize];
                        //printf("%lu\n", sizeof(mb));
                        memset(&mb, 0, sizeof(mb));
                        while((bytes = read(f, &word, sizeof(word))) > 0) {
                            //printf("%s\n", word);
                            strcat(mb, word);
                            memset(&word, 0, sizeof(word));
                        }
                        //printf("%s\n", mb);
                        code = 200;
                        //printf("%s", mb);
                        char response[fsize + 2048];
                        
                        httpresponse(acceptfd, code, fsize, response, mb);
                        close(f);
                        continue;
                    }
                }
            } else if(strcmp(r_type, "HEAD") == 0) {
                if(S_ISDIR(path_stat.st_mode)) {
                    code = 403;
                    httpresponse(acceptfd, code, strlen(getMessage(code)), response, getMessage(code));
                } else if(!S_ISREG(path_stat.st_mode)){
                    code = 404;
                    httpresponse(acceptfd, code, strlen(getMessage(code)), response, getMessage(code));
                } else {
                    f = open(URI, O_RDONLY);
                    if(f < 0) {
                        code = 403;
                        httpresponse(acceptfd, code, strlen(getMessage(code)), response, getMessage(code));
                    } else {
                        off_t fsize = path_stat.st_size;
                        code = 200;
                        httpresponse(acceptfd, code, fsize, response, "");
                    }
                    close(f);
                    continue;
                }
            } else {
                code = 501;
                httpresponse(acceptfd, code, strlen(getMessage(code)), response, getMessage(code));
            }
            close(acceptfd);   
        }
        memset(&buffer, 0, sizeof(buffer));
        memset(&response, 0, sizeof(response));
        memset(&request, 0, sizeof(request));
        memset(&r_type, 0, sizeof(r_type));
        memset(&c_l, 0, sizeof(c_l));
        memset(&URI, 0, sizeof(URI));
        memset(&path_stat, 0, sizeof(path_stat));
        
    }
}
