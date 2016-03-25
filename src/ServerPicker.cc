#include "ServerPicker.h"

//ServerPicker::ServerPicker()
//{
  //  init();
//}

int ServerPicker::init()
{
    server_num = 0;

    FILE* fp;
    fp = fopen("../conf/downloader.conf","r");
    if(NULL == fp)
    {
        printf("Open file failed!\n");
        return -1;
    }
    char* line = NULL;
    char ip[16];
    char port[16];
    memset(ip,'\0',sizeof(ip));
    memset(port,'\0',sizeof(port));

    size_t len = 0;

    unsigned port_num;
    
    while(getline(&line,&len,fp) != -1)
    {
        //puts(line);
        //printf("The len of line %d\n", strlen(line));
        if(strlen(line) < 5)
            continue;
        char* index1 = strchr(line,':');

        int num1 = index1 - line;
        //printf("%d\n", num1);
        strncpy(ip,line,num1);
        int num2 = strlen(line) - num1 - 1;
        //printf("%d\n", num2);
        strncpy(port,index1+1,num2);
        //printf("%s\n", ip );
        //printf("%s\n", port);

        port_num = atoi(port);
        //printf("%d\n", port2);
        ServerInfo* server = new ServerInfo(ip,port_num);
        all_servers[server_num++] = server;
    }
    fclose(fp);
    return 0;

}

bool ServerPicker::getServerInfo(ServerInfo& server)
{
    srand (time(NULL));
    int index  = rand() % server_num;
    //printf("The choose server is %d\n", index);
    //int index = 0;
    ServerInfo* p_server = all_servers[index];

    strcpy(server.ip, p_server->ip);
    server.port = p_server->port;
    //server.timeout = p_server->timeout;

    //printf("%s : %d  %d\n", server.ip, server.port,server.timeout);
    //printf("%d\n", server.port);
    //printf("%d\n", server.timeout);

    return true;
}
