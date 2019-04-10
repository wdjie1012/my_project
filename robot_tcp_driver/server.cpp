
#include <sever.h>
#include <fstream>

using namespace std;

int picLen=0;

int num=0;
int i=0;
char inputBuffer[256] = {};
int port=8700;
bool runFlag;
int forClientSockfd = 0;
int sockfd = 0;       //sockfd套接字
int recvbytes=0;                          //判断是否断开连接
struct sockaddr_in serverInfo,clientInfo; // clientInfo连接实体地址
socklen_t addrlen = sizeof(clientInfo);

union data
{
    int32_t pic_length;
    char tab[4];
}pic_data;

void server();
RecvContainer *Receive();

std::vector<RecvContainer> buffer_pool_;

RecvContainer *recv_container_ptr_;


bool SeverInit()
{
    /**
 * 1.创建套接字
 */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    /**
     * 2.绑定
     */
    bzero(&serverInfo, sizeof(serverInfo));
    serverInfo.sin_family = PF_INET;
    serverInfo.sin_addr.s_addr = INADDR_ANY;
    serverInfo.sin_port = htons(port);
    bind(sockfd, (struct sockaddr *) &serverInfo, sizeof(serverInfo));
    /**
     * 3.监听
     */
    listen(sockfd,5);

    recv_container_ptr_=new RecvContainer();

    thread t1(server);
    t1.detach();



    return true;

}

void server() {

    while (1) {
        forClientSockfd = accept(sockfd, (struct sockaddr *) &clientInfo, &addrlen);
        runFlag=true;
        while(runFlag) {
            RecvContainer *container_ptr=Receive();
            if(container_ptr)
            {
                buffer_pool_.push_back(*container_ptr);
            }
            usleep(100);
        }
    }
}

RecvContainer *Receive()
{
    //read the file
    char ctl_mes[7]={};
    ifstream in("/home/wdjie/Downloads/thebird2.jpg");
    in.seekg(0,in.end);   //the end
    int length=in.tellg();//calculate the length
    in.seekg(0,in.beg);   //the start
    char buffer[length];
    in.read(buffer,length);


    //define the array to send
    //auto sendPic=new char[length+4];
    char sendPic[length+4];
    char tab[4];
    pic_data.pic_length=length;
    tab[0]=pic_data.tab[3];
    tab[1]=pic_data.tab[2];
    tab[2]=pic_data.tab[1];
    tab[3]=pic_data.tab[0];
    int j=0;
    for(i=0;i< sizeof(tab);i++)
        sendPic[i]=tab[i];
    for(i= sizeof(tab);i< (sizeof(buffer)+ sizeof(tab));i++)
        sendPic[i]=buffer[j++];


    RecvContainer *recv_container=recv_container_ptr_;
    memset(ctl_mes, 0, sizeof(ctl_mes));
    recvbytes = recv(forClientSockfd, ctl_mes, sizeof(ctl_mes), 0);


    /**
     * 异常情况退出
     */
    if(recvbytes==0||recvbytes==-1){
        runFlag=false;
        //return 0;
    }

    if(ctl_mes[0]=='A'&&ctl_mes[1]=='A') {
        send(forClientSockfd, sendPic, sizeof(sendPic), 0);
    }

    else if(ctl_mes[0]=='C'&&ctl_mes[1]=='T') {
        cout<<"************************"<<ctl_mes[4]<<endl;
        /**
         * 处理粘包
         */
        /*if (ctl_mes[2] == ctl_mes[3] == ctl_mes[4] == 'F' || ctl_mes[6] == 'E') {
            ctl_mes[7] = '\0';
        }*/
        /**
         * 接口
         */
        u_char mes = 0;
        switch (ctl_mes[4]) {
            case 'U':
                mes = 1;
                break;
            case 'D':
                mes = 2;
                break;
            case 'L':
                mes = 3;
                break;
            case 'R':
                mes = 4;
                break;
            case 'S':
                mes = 5;
                break;
            default:
                mes = 0;
        }
        printf("recv_container_ptr_=%d\n", recv_container->direct_cmd);
        recv_container->direct_cmd = mes;
    }
        return recv_container;

}


bool Take(RecvContainer *recv_container)
{
    if(!buffer_pool_.size())return false;
    else{
        printf("\nbuffer_pool_.size=%d\n",buffer_pool_.size());
        *recv_container=buffer_pool_.front();
        printf("run Take%d\n",recv_container->direct_cmd);
        buffer_pool_.erase(buffer_pool_.begin());
        return true;
    }

}







