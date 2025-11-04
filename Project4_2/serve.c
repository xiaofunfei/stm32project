#include <header.h>


typedef enum {
    MY_LOG_ERROR,
    MY_LOG_WARNING,
    MY_LOG_INFO,
    MY_LOG_DEBUG
} LOG_Status;

typedef  struct  user_s{
    int net_fd;
    time_t last_time;

}user_t;

int  MY_LOG_LEVEL =  MY_LOG_DEBUG;

void get_date(char * buf){
    time_t now;
    time(&now);  
    struct tm *local_time = localtime(&now);
    int year    = local_time->tm_year + 1900; 
    int month   = local_time->tm_mon  + 1;    
    int day     = local_time->tm_mday;        
    int hour    = local_time->tm_hour;        
    int minute  = local_time->tm_min;         
    int second  = local_time->tm_sec;         
    sprintf(buf, "[%04d-%02d-%02d %02d:%02d:%02d]",
            year, month, day, hour, minute, second);
}


void my_log(const char *file,const char * funcname, int linenum, char *message, int level){

    if(level > MY_LOG_LEVEL){
        return;
    }

    int fd = open("error.log", O_RDWR|O_CREAT, 0666);
    close(fd);
    fd = open("warning.log", O_RDWR|O_CREAT, 0666);
    close(fd);
    fd = open("info.log", O_RDWR|O_CREAT, 0666);
    close(fd);
    fd = open("debug.log", O_RDWR|O_CREAT, 0666);
    close(fd);

    char date[100] = {0};
    get_date(date);

    if(level == MY_LOG_ERROR){
        char buf[1024] = {0};
        sprintf(buf, "%s:[ERROR]: [%s][%s][%d] : %s \n", date, file, funcname, linenum, message);
        int fd = open("error.log", O_RDWR|O_APPEND);
        write(fd, buf, strlen(buf));
        close(fd);
        fd = open("warning.log", O_RDWR|O_APPEND);
        write(fd, buf, strlen(buf));
        close(fd);
        fd = open("info.log", O_RDWR|O_APPEND);
        write(fd, buf, strlen(buf));
        close(fd);
        fd = open("debug.log", O_RDWR|O_APPEND);
        write(fd, buf, strlen(buf));
        close(fd);
    }else if(level == MY_LOG_WARNING){
        char buf[1024] = {0};
        sprintf(buf, "%s:[WARING]: [%s][%s][%d] : %s \n", date, file, funcname, linenum, message);

        int fd = open("warning.log", O_RDWR|O_APPEND);
        write(fd, buf, strlen(buf));
        close(fd);
        fd = open("info.log", O_RDWR|O_APPEND);
        write(fd, buf, strlen(buf));
        close(fd);
        fd = open("debug.log", O_RDWR|O_APPEND);
        write(fd, buf, strlen(buf));
        close(fd);
    }else if(level == MY_LOG_INFO){
        char buf[1024] = {0};
        sprintf(buf, "%s:[INFO]: [%s][%s][%d] : %s \n", date, file, funcname, linenum, message);

        int fd = open("info.log", O_RDWR|O_APPEND);
        write(fd, buf, strlen(buf));
        close(fd);
        fd = open("debug.log", O_RDWR|O_APPEND);
        write(fd, buf, strlen(buf));
        close(fd);
    }else if(level == MY_LOG_DEBUG){
        char buf[1024] = {0};
        sprintf(buf, "%s:[DEBUG]: [%s][%s][%d] : %s \n", date, file, funcname, linenum, message);

        int fd = open("debug.log", O_RDWR|O_APPEND);
        write(fd, buf, strlen(buf));
        close(fd);
    }
}

int getparameter(char *key, char *value){

    FILE * file = fopen("config.ini", "r");
    while(1){
        char line[100];
        bzero(line, sizeof(line));
        char *res = fgets(line, sizeof(line), file);
        if(res == NULL){
            char buf[] = "没有要找的内容 \n";
            memcpy(value, buf, strlen(buf));
            return -1;
        }
        char *line_key = strtok(line, "=");
        if(strcmp(key, line_key) == 0){
            char *line_value = strtok(NULL, "=");
            memcpy(value, line_value, strlen(line_value));
            return 0;
        }
    }

    return 0;
}
void loadloglevel(){
    char log_str[100] = {0};
    getparameter("log", log_str);
    if(strstr(log_str, "error")){
        MY_LOG_LEVEL = MY_LOG_ERROR;
    }else if(strstr(log_str, "warning")){
        MY_LOG_LEVEL = MY_LOG_WARNING;
    }else if(strstr(log_str, "info")){
        MY_LOG_LEVEL = MY_LOG_INFO;
    }else if(strstr(log_str, "debug")){
        MY_LOG_LEVEL = MY_LOG_DEBUG;
    }
}

int dealRequset(int net_fd, char *buf){
    int index = 0;
    while(buf[index] != '/'){
        index=index+1;
        if(index>1000){
            break;
        }
    }
    if(buf[index+1] == 'x'){

        char parameter[200] = {0};
        //POST /x HTTP/1.1 
        // 
        //=1:2000,2:1700=
        char *start = strstr(buf, "=");
        start = strstr(start, "1");
        char *end = strchr(start, '=');
        ssize_t len = end-start;
        strncpy(parameter, start, len);

        FILE *file = fopen("data.txt", "a");
        fprintf(file, "%s\n", parameter);
        fflush(file);
        fclose(file);

        return 0;
    }else if(buf[index+1]=='y'){
        //POST /y HTTP/1.1 
        // 
        //=1:2000,2:1700==1:2000,2:1700==1:2000,2:1700==1:2000,2:1700=    (共十组)

        int tag = 0;

        while(1){
            if(buf[tag] == '='){
                break;
            }
            tag++;
        }

        while(tag < strlen(buf)-1){

            int nums = 0;

            char temp[100] = {0};
            while(nums < 2){
                if(buf[tag] == '='){
                    nums++;
                    tag++;
                    continue;
                }
                temp[strlen(temp)] = buf[tag];
                tag++;
            }

            FILE *file = fopen("data.txt", "a");
            fprintf(file, "%s\n", temp);
            fflush(file);
            fclose(file);
        }

    }else if(buf[index+1]=='1'){

        char buf[409600] = {0};
        char *str1 = "HTTP/1.1 200 OK\r\n\r\n";
        strncpy(buf, str1, strlen(str1));

        FILE *file = fopen("data.txt", "r");

        buf[strlen(buf)]='[';

        while(1){
            char line[100] = {0};
            char * pRet = fgets(line, sizeof(line), file);
            if(pRet == NULL){
                break;
            }
            buf[strlen(buf)]='\"';
            strncpy(buf+strlen(buf), line, strlen(line));
            buf[strlen(buf)-1]='\"';
            buf[strlen(buf)]=',';
        }
        if(strlen(buf)>25){
            buf[strlen(buf)-1]=']';
        }else{
            char *str1 = "HTTP/1.1 200 OK\r\n\r\n[]";
            sprintf(buf, "%s", str1);
        }

        send(net_fd, buf, strlen(buf), 0);

        return 1;
    }else if(buf[index+1] =='2'){
        FILE *file = fopen("data.txt", "w");
        fclose(file);

        char *str1 = "HTTP/1.1 200 OK\r\n\r\n[]";
        send(net_fd, str1, strlen(str1), 0);
        return 2;
    }

    return 0;
}


int main(int argc, char *argv[argc]){



    loadloglevel();
    char ip[100];
    bzero(ip, sizeof(ip));
    getparameter("ip", ip);
    char port[100];
    bzero(port, sizeof(port));
    getparameter("port", port);

    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(socket_fd == -1){
        my_log(__FILE__, __FUNCTION__, __LINE__, "socket", MY_LOG_ERROR);
    }

    int reuse = 1;
    setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    struct sockaddr_in sock;
    sock.sin_family = AF_INET;
    sock.sin_port = htons(atoi(port));
    sock.sin_addr.s_addr = inet_addr(ip);
    int ret = bind(socket_fd, (struct sockaddr *)&sock, sizeof(sock));
    if(ret == -1){
        my_log(__FILE__, __FUNCTION__, __LINE__, "bind", MY_LOG_ERROR);
    }

    listen(socket_fd, 10);

    user_t list[100];
    memset(list, 0, sizeof(list));
    int size = 0;

    fd_set set;
    FD_ZERO(&set);
    FD_SET(socket_fd, &set);

    int max = socket_fd;
    while(1){

        fd_set temp; 
        memcpy(&temp, &set, sizeof(fd_set));

        struct timeval tv;
        tv.tv_sec = 1;
        tv.tv_usec = 0;
        int num = select(max+1, &temp, NULL, NULL, &tv);
        my_log(__FILE__, __FUNCTION__, __LINE__, "select", MY_LOG_DEBUG);



        if(num == 0){
            time_t now_time;
            time(&now_time);
            for(int i=0; i<size; i++){
                if(now_time - list[i].last_time > 30){
                    FD_CLR(list[i].net_fd, &set);
                    close(list[i].net_fd);
                    my_log(__FILE__, __FUNCTION__, __LINE__, "timeout_close", MY_LOG_WARNING);

                    for(int k=i; k<size; k++){
                        list[k] = list[k+1];
                    }
                    size--;
                    i--;
                }
            }

        }

        if(FD_ISSET(socket_fd, &temp)){
            int net_fd = accept(socket_fd, NULL, NULL);
            my_log(__FILE__, __FUNCTION__, __LINE__, "new_linked", MY_LOG_WARNING);
            list[size].net_fd = net_fd;
            time(&list[size].last_time);
            size++;

            FD_SET(net_fd, &set);
            max = net_fd>max? net_fd: max;
        }

        for(int  i=0; i<size; i++){

            int net_fd = list[i].net_fd;

            if(FD_ISSET(net_fd, &temp)){

                char buf[4096] = {0};
                ssize_t ret = recv(net_fd, buf, sizeof(buf), 0);
                my_log(__FILE__, __FUNCTION__, __LINE__, "recv", MY_LOG_INFO);
                if(ret == 0){
                    FD_CLR(net_fd, &set);
                    for(int k=i; k<size; k++){
                        list[k] = list[k+1];
                    }
                    my_log(__FILE__, __FUNCTION__, __LINE__, "close", MY_LOG_INFO);
                    size--;
                    i--;
                    continue;
                }

                time(&list[i].last_time);
                int dealRet = dealRequset(list[i].net_fd, buf);

                if(dealRet == 1 || dealRet == 2){
                    FD_CLR(net_fd, &set);
                    close(net_fd);
                    for(int k=i; k<size; k++){
                        list[k] = list[k+1];
                    }
                    my_log(__FILE__, __FUNCTION__, __LINE__, "close", MY_LOG_INFO);
                    size--;
                    i--;
                }
            }
        }
    }
    close(socket_fd);
    return 0;
}
