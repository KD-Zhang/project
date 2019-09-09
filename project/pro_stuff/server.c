#include<stdio.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/select.h>
#include <pthread.h>

#include "common.h"

sqlite3 *db;  //仅服务器使用
int flags = 0;


void get_system_time(char* timedata)
{
	time_t t;
	struct tm *tp;

	time(&t);
	tp = localtime(&t);
	sprintf(timedata,"%d-%d-%d %d:%d:%d",tp->tm_year+1900,tp->tm_mon+1,\
			tp->tm_mday,tp->tm_hour,tp->tm_min,tp->tm_sec);
	return ;
}

/******************************************************
*函数名：history_init
*参   数：线程处理结构体，用户处理内容buf
*******************************************************/
void history_init(MSG *msg,char *buf)
{
	//获取当前时间--封装sql命令---将buf用户的操作记录插入到历史记录的表当中
	int nrow,ncolumn;
	char *errmsg, **resultp;
	char sqlhistory[DATALEN] = {0};
	char timedata[DATALEN] = {0};

	get_system_time(timedata);
 
	sprintf(sqlhistory,"insert into historyinfo values ('%s','%s','%s');",timedata,msg->username,buf);
	if(sqlite3_exec(db,sqlhistory,NULL,NULL,&errmsg)!= SQLITE_OK){
		printf("%s.\n",errmsg);
		printf("insert historyinfo failed.\n");
	}else{
		printf("insert historyinfo success.\n");
	}
}


int process_user_or_admin_login_request(int acceptfd,MSG *msg)
{
	//封装sql命令，表中查询用户名和密码－存在－登录成功－发送响应－失败－发送失败响应	
	char sql[DATALEN] = {0};
	char *errmsg;
	char **result;
	int nrow,ncolumn;

	msg->info.usertype =  msg->usertype;
	strcpy(msg->info.name,msg->username);
	strcpy(msg->info.passwd,msg->passwd);
	
	printf("usrtype: %#x-----usrname: %s---passwd: %s.\n",msg->info.usertype,msg->info.name,msg->info.passwd);
	sprintf(sql,"select * from usrinfo where usertype=%d and name='%s' and passwd='%s';",msg->info.usertype,msg->info.name,msg->info.passwd);
	if(sqlite3_get_table(db,sql,&result,&nrow,&ncolumn,&errmsg) != SQLITE_OK){
		printf("---****----%s.\n",errmsg);		
	}else{

		if(nrow == 0){
			strcpy(msg->recvmsg,"name or passwd failed.\n");
			send(acceptfd,msg,sizeof(MSG),0);
		}else{
			strcpy(msg->recvmsg,"OK");
			send(acceptfd,msg,sizeof(MSG),0);
		}
	}
	return 0;	
}

int process_user_modify_request(int acceptfd,MSG *msg)
{
	printf("------------%s-----------%d.\n",__func__,__LINE__);
	int nrow,ncolumn;
	char *errmsg, **resultp;
	char sql[DATALEN] = {0};	
	char historybuf[DATALEN] = {0};

	switch (msg->recvmsg[0])
	{
		case 'P':
			sprintf(sql,"update usrinfo set phone='%s' where staffno=%d;",msg->info.phone,msg->info.no);
			sprintf(historybuf,"%s修改工号为%d的电话为%s",msg->username,msg->info.no,msg->info.phone);
			break;
		case 'D':
			sprintf(sql,"update usrinfo set addr='%s' where staffno=%d;",msg->info.addr, msg->info.no);
			sprintf(historybuf,"%s修改工号为%d的家庭住址为%s",msg->username,msg->info.no,msg->info.addr);
			break;	
		case 'M':
			sprintf(sql,"update usrinfo set passwd='%s' where staffno=%d;",msg->info.passwd, msg->info.no);
			sprintf(historybuf,"%s修改工号为%d的密码为%s",msg->username,msg->info.no,msg->info.passwd);
			break;
	}
	printf("msgtype :%#x--usrtype: %#x--usrname: %s-passwd: %s.\n",msg->msgtype,msg->info.usertype,msg->info.name,msg->info.passwd);
	printf("msg->info.no :%d\t msg->info.addr %s\t msg->info.phone: %s.\n",msg->info.no,msg->info.addr,msg->info.phone);

	//调用sqlite3_exec执行sql命令
	if(sqlite3_exec(db,sql,NULL,NULL,&errmsg) != SQLITE_OK){
		printf("%s.\n",errmsg);
		sprintf(msg->recvmsg,"数据库修改失败！%s\n", errmsg);
	}else{
		printf("the database is updated successfully.\n");
		sprintf(msg->recvmsg, "数据库修改成功!\n");
		history_init(msg,historybuf);
	}

	//通知用户信息修改成功
	send(acceptfd,msg,sizeof(MSG),0);

	printf("------%s.\n",historybuf);
	return 0;

}



int process_user_query_request(int acceptfd,MSG *msg)
{
	printf("------------%s-----------%d.\n",__func__,__LINE__);
	//检查msg->flags--->封装sql命令－查找历史记录表－回调函数－发送查询结果－发送结束标志

	int i = 0,j = 0;
	char sql[DATALEN] = {0};
	char **resultp;
	int nrow,ncolumn;
	char *errmsg;

	sprintf(sql,"select * from usrinfo where name='%s';",msg->username);
	if(sqlite3_get_table(db, sql, &resultp,&nrow,&ncolumn,&errmsg) != SQLITE_OK){
		printf("%s.\n",errmsg);
	}else{
		printf("searching.....\n");	
		for(i = 0; i < ncolumn; i ++){
			printf("%-8s ",resultp[i]);
		}
		puts("");
		puts("======================================================================================");
				
		int index = ncolumn;
		for(i = 0; i < nrow; i ++){
			printf("%s    %s     %s     %s     %s     %s     %s     %s     %s     %s     %s.\n",resultp[index+ncolumn-11],resultp[index+ncolumn-10],\
				resultp[index+ncolumn-9],resultp[index+ncolumn-8],resultp[index+ncolumn-7],resultp[index+ncolumn-6],resultp[index+ncolumn-5],\
				resultp[index+ncolumn-4],resultp[index+ncolumn-3],resultp[index+ncolumn-2],resultp[index+ncolumn-1]);
				
			sprintf(msg->recvmsg,"%s,    %s,    %s,    %s,    %s,    %s,    %s,    %s,    %s,    %s,    %s;",resultp[index+ncolumn-11],resultp[index+ncolumn-10],\
				resultp[index+ncolumn-9],resultp[index+ncolumn-8],resultp[index+ncolumn-7],resultp[index+ncolumn-6],resultp[index+ncolumn-5],\
				resultp[index+ncolumn-4],resultp[index+ncolumn-3],resultp[index+ncolumn-2],resultp[index+ncolumn-1]);
			send(acceptfd,msg,sizeof(MSG),0);
			//}
			usleep(1000);
			puts("======================================================================================");
			index += ncolumn;
		}

		sqlite3_free_table(resultp);
		printf("sqlite3_get_table successfully.\n");
	}

}


int process_admin_modify_request(int acceptfd,MSG *msg)
{
	printf("------------%s-----------%d.\n",__func__,__LINE__);
	int nrow,ncolumn;
	char *errmsg, **resultp;
	char sql[DATALEN] = {0};	
	char historybuf[DATALEN] = {0};

	switch (msg->recvmsg[0])
	{
		case 'N':
			sprintf(sql,"update usrinfo set name='%s' where staffno=%d;",msg->info.name, msg->info.no);
			sprintf(historybuf,"%s修改工号为%d的用户名为%s",msg->username,msg->info.no,msg->info.name);
			break;
		case 'A':
			sprintf(sql,"update usrinfo set age=%d where staffno=%d;",msg->info.age, msg->info.no);
			sprintf(historybuf,"%s修改工号为%d的年龄为%d",msg->username,msg->info.no,msg->info.age);
			break;
		case 'P':
			sprintf(sql,"update usrinfo set phone='%s' where staffno=%d;",msg->info.phone,msg->info.no);
			sprintf(historybuf,"%s修改工号为%d的电话为%s",msg->username,msg->info.no,msg->info.phone);
			break;
		case 'D':
			sprintf(sql,"update usrinfo set addr='%s' where staffno=%d;",msg->info.addr, msg->info.no);
			sprintf(historybuf,"%s修改工号为%d的家庭住址为%s",msg->username,msg->info.no,msg->info.addr);
			break;
		case 'W':
			sprintf(sql,"update usrinfo set work='%s' where staffno=%d;",msg->info.work, msg->info.no);
			sprintf(historybuf,"%s修改工号为%d的职位为%s",msg->username,msg->info.no,msg->info.work);
			break;
		case 'T':
			sprintf(sql,"update usrinfo set date='%s' where staffno=%d;",msg->info.date, msg->info.no);
			sprintf(historybuf,"%s修改工号为%d的入职日期为%s",msg->username,msg->info.no,msg->info.date);
			break;
		case 'L':
			sprintf(sql,"update usrinfo set level=%d where staffno=%d;",msg->info.level, msg->info.no);
			sprintf(historybuf,"%s修改工号为%d的评级为%d",msg->username,msg->info.no,msg->info.level);
			break;
		case 'S':
			sprintf(sql,"update usrinfo set salary=%.2f where staffno=%d;",msg->info.salary, msg->info.no);
			sprintf(historybuf,"%s修改工号为%d的工资为%.2f",msg->username,msg->info.no,msg->info.salary);
			break;
		case 'M':
			sprintf(sql,"update usrinfo set passwd='%s' where staffno=%d;",msg->info.passwd, msg->info.no);
			sprintf(historybuf,"%s修改工号为%d的密码为%s",msg->username,msg->info.no,msg->info.passwd);
			break;
	}
	
	//调用sqlite3_exec执行sql命令
	if(sqlite3_exec(db,sql,NULL,NULL,&errmsg) != SQLITE_OK){
		printf("%s.\n",errmsg);
		sprintf(msg->recvmsg,"数据库修改失败！%s", errmsg);
	}else{
		printf("the database is updated successfully.\n");
		sprintf(msg->recvmsg, "数据库修改成功!");
		history_init(msg,historybuf);
	}

	//通知用户信息修改成功
	send(acceptfd,msg,sizeof(MSG),0);

	printf("------%s.\n",historybuf);
	return 0;
}





int process_admin_adduser_request(int acceptfd,MSG *msg)
{
	//封装sql命令--- 插入一个用户信息到数据库－如果插入成功则发送ＯＫ通知客户端--否则发送失败---同时插入历史记录数据
	char sql[DATALEN] = {0};
	char buf[DATALEN] = {0};
	char *errmsg;

	printf("%d\t %d\t %s\t %s\t %d\n %s\t %s\t %s\t %s\t %d\t %f.\n",msg->info.no,msg->info.usertype,msg->info.name,msg->info.passwd,\
		msg->info.age,msg->info.phone,msg->info.addr,msg->info.work,\
		msg->info.date,msg->info.level,msg->info.salary);
	
	sprintf(sql,"insert into usrinfo values(%d,%d,'%s','%s',%d,'%s','%s','%s','%s',%d,%f);",\
		msg->info.no,msg->info.usertype,msg->info.name,msg->info.passwd,\
		msg->info.age,msg->info.phone,msg->info.addr,msg->info.work,\
		msg->info.date,msg->info.level,msg->info.salary);
	
	if(sqlite3_exec(db,sql,NULL,NULL,&errmsg)!= SQLITE_OK){
		printf("----------%s.\n",errmsg);
		strcpy(msg->recvmsg,"failed");
		send(acceptfd,msg,sizeof(MSG),0);
		return -1;
	}else{
		strcpy(msg->recvmsg,"OK");
		send(acceptfd,msg,sizeof(msg),0);
		printf("%s register success.\n",msg->info.name);
	}

	sprintf(buf,"管理员%s添加了%s用户",msg->username,msg->info.name);
	history_init(msg,buf);

	return 0;
}



int process_admin_deluser_request(int acceptfd,MSG *msg)
{
	printf("------------%s-----------%d.\n",__func__,__LINE__);
	//封装sql命令--- 插入一个用户信息到数据库－如果插入成功则发送ＯＫ通知客户端--否则发送失败---同时插入历史记录数据
	char sql[DATALEN] = {0};
	char buf[DATALEN] = {0};
	char *errmsg;

	printf("msg->info.no :%d\t msg->info.name: %s.\n",msg->info.no,msg->info.name);
	
	sprintf(sql,"delete from usrinfo where staffno=%d and name='%s';",msg->info.no,msg->info.name);	
	if(sqlite3_exec(db,sql,NULL,NULL,&errmsg)!= SQLITE_OK){
		printf("----------%s.\n",errmsg);
		strcpy(msg->recvmsg,"failed");
		send(acceptfd,msg,sizeof(MSG),0);
		return -1;
	}else{
		strcpy(msg->recvmsg,"OK");
		send(acceptfd,msg,sizeof(msg),0);
		printf("%s deluser %s success.\n",msg->info.name,msg->info.name);
	}

	sprintf(buf,"管理员%s删除了%s用户",msg->username,msg->info.name);
	history_init(msg,buf);

	return 0;
}


int process_admin_query_request(int acceptfd,MSG *msg)
{
	printf("------------%s-----------%d.\n",__func__,__LINE__);
	//检查msg->flags--->封装sql命令－查找历史记录表－回调函数－发送查询结果－发送结束标志

	
	int i = 0,j = 0;
	char sql[DATALEN] = {0};
	char **resultp;
	int nrow,ncolumn;
	char *errmsg;

	if(msg->flags == 1){
		sprintf(sql,"select * from usrinfo where name='%s';",msg->info.name);
	}else{
		sprintf(sql,"select * from usrinfo;");
	}
	
	if(sqlite3_get_table(db, sql, &resultp,&nrow,&ncolumn,&errmsg) != SQLITE_OK){
		printf("%s.\n",errmsg);
	}else{
		printf("searching.....\n");
		printf("ncolumn :%d\tnrow :%d.\n",ncolumn,nrow);
		
		for(i = 0; i < ncolumn; i ++){
			printf("%-8s ",resultp[i]);
		}
		puts("");
		puts("=============================================================");
		
		int index = ncolumn;
		for(i = 0; i < nrow; i ++){
				printf("%s    %s     %s     %s     %s     %s     %s     %s     %s     %s     %s.\n",resultp[index+ncolumn-11],resultp[index+ncolumn-10],\
				resultp[index+ncolumn-9],resultp[index+ncolumn-8],resultp[index+ncolumn-7],resultp[index+ncolumn-6],resultp[index+ncolumn-5],\
				resultp[index+ncolumn-4],resultp[index+ncolumn-3],resultp[index+ncolumn-2],resultp[index+ncolumn-1]);
				
			sprintf(msg->recvmsg,"%s,    %s,    %s,    %s,    %s,    %s,    %s,    %s,    %s,    %s,    %s;",resultp[index+ncolumn-11],resultp[index+ncolumn-10],\
				resultp[index+ncolumn-9],resultp[index+ncolumn-8],resultp[index+ncolumn-7],resultp[index+ncolumn-6],resultp[index+ncolumn-5],\
				resultp[index+ncolumn-4],resultp[index+ncolumn-3],resultp[index+ncolumn-2],resultp[index+ncolumn-1]);
			send(acceptfd,msg,sizeof(MSG),0);
			//}
			usleep(1000);
			puts("=============================================================");
			index += ncolumn;
		}
		
		if(msg->flags != 1){  //全部查询的时候不知道何时结束，需要手动发送结束标志位，但是按人名查找不需要
			//通知对方查询结束了
			strcpy(msg->recvmsg,"over*");
			send(acceptfd,msg,sizeof(MSG),0);
		}
		
		sqlite3_free_table(resultp);
		printf("sqlite3_get_table successfully.\n");
	}



}


//回调函数
int history_callback(void *arg, int ncolumn, char **f_value, char **f_name)
{
	int i = 0;
	MSG *msg= (MSG *)arg;
	int acceptfd = msg->flags;

	if(flags == 0){
		for(i = 0; i < ncolumn; i++){
			printf("%-11s", f_name[i]);
		}
		putchar(10);
		flags = 1;
	}

	for(i = 0; i < ncolumn; i+=3)
	{
		printf("%s-%s-%s",f_value[i],f_value[i+1],f_value[i+2]);
		sprintf(msg->recvmsg,"%s---%s---%s.\n",f_value[i],f_value[i+1],f_value[i+2]);
		send(acceptfd,msg,sizeof(MSG),0);
		usleep(1000);
	}
	puts("");

	return 0;
}


int process_admin_history_request(int acceptfd,MSG *msg)
{
	printf("------------%s-----------%d.\n",__func__,__LINE__);
	//封装sql命令－查找历史记录表－回调函数－发送查询结果－发送结束标志
	char sql[DATALEN] = {0};
	char *errmsg;
	msg->flags = acceptfd; //临时保存通信的文件描述符
	
	sprintf(sql,"select * from historyinfo;");
	if(sqlite3_exec(db,sql,history_callback,(void *)msg,&errmsg) != SQLITE_OK){
		printf("%s.\n",errmsg); 	
	}else{
		printf("query history record done.\n");
	}

	//通知对方查询结束了
	strcpy(msg->recvmsg,"over*");
	send(acceptfd,msg,sizeof(MSG),0);

	 flags = 0; //记得将全局变量修改为原来的状态
}


int process_client_quit_request(int acceptfd,MSG *msg)
{

}


int process_client_request(int acceptfd,MSG *msg)
{
	printf("--%s--%d--\n", __func__,__LINE__);
	switch (msg->msgtype)
	{
		case USER_LOGIN:
		case ADMIN_LOGIN:
			process_user_or_admin_login_request(acceptfd,msg);
			break;
		case USER_MODIFY:
			process_user_modify_request(acceptfd,msg);
			break;
		case USER_QUERY:
			process_user_query_request(acceptfd,msg);
			break;
		case ADMIN_MODIFY:
			process_admin_modify_request(acceptfd,msg);
			break;
		case ADMIN_ADDUSER:
			process_admin_adduser_request(acceptfd,msg);
			break;
		case ADMIN_DELUSER:
			process_admin_deluser_request(acceptfd,msg);
			break;
		case ADMIN_QUERY:
			process_admin_query_request(acceptfd,msg);
			break;
		case ADMIN_HISTORY:
			process_admin_history_request(acceptfd,msg);
			break;
		case QUIT:
			process_client_quit_request(acceptfd,msg);
			break;
		default:
			break;
	}

}


int main(int argc, const char *argv[])
{
	int listenfd, connfd, sockfd, i, maxi, maxfd;
	int nready, client[FD_SETSIZE];
	ssize_t n;
	fd_set rset, allset;
	struct sockaddr_in serveraddr,  clientaddr;
	socklen_t cli_len = sizeof(clientaddr);
	char buf[64];
	char str[INET_ADDRSTRLEN];

	MSG msg;
	char *errmsg;

	if(sqlite3_open(STAFF_DATABASE,&db) != SQLITE_OK){
		printf("%s.\n",sqlite3_errmsg(db));
	}else{
		printf("the database open success.\n");
	}

	if(sqlite3_exec(db,"create table usrinfo(staffno integer,usertype integer,name text,passwd text,age integer,phone text,addr text,work text,date text,level integer,salary REAL);",NULL,NULL,&errmsg)!= SQLITE_OK){
		printf("%s.\n",errmsg);
	}else{
		printf("create usrinfo table success.\n");
	}

	if(sqlite3_exec(db,"create table historyinfo(time text,name text,words text);",NULL,NULL,&errmsg)!= SQLITE_OK){
		printf("%s.\n",errmsg);
	}else{ 
		printf("create historyinfo table success.\n");
	}

	listenfd = socket(AF_INET,SOCK_STREAM, 0);
	if(listenfd == -1){
		perror("socket failed.\n");
		exit(-1);
	}
	printf("listenfd :%d.\n",listenfd); 

	
	int b_reuse = 1;
	setsockopt (sockfd, SOL_SOCKET, SO_REUSEADDR, &b_reuse, sizeof (int));
	
	//填充网络结构体
	memset(&serveraddr,0,sizeof(serveraddr));
	memset(&clientaddr,0,sizeof(clientaddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port   = htons(5001);
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);


	if(bind(listenfd, (const struct sockaddr *)&serveraddr,sizeof(serveraddr)) == -1){
		printf("bind failed.\n");
		exit(-1);
	}

	if(listen(listenfd,10) == -1){
		printf("listen failed.\n");
		exit(-1);
	}

	maxfd = listenfd;
	maxi = -1;
	for(i = 0; i < FD_SETSIZE; i++)
		client[i] = -1;

	FD_ZERO(&allset);
	FD_SET(listenfd,&allset);

	for(;;){
		rset = allset;
		nready = select(maxfd + 1, &rset, NULL, NULL, NULL);
		if(nready < 0)
			perror("select error");
		if(FD_ISSET(listenfd, &rset)){
			connfd = accept(listenfd, (struct sockaddr*)&clientaddr, &cli_len);
			printf("received from %s at PORT %d\n",
					inet_ntop(AF_INET, &clientaddr.sin_addr, str, sizeof(str)),
					ntohs(clientaddr.sin_port));
			for(i = 0; i < FD_SETSIZE; i++){
				if(client[i] < 0){
					client[i] = connfd;
					break;
				}
			}
			FD_SET(connfd, &allset) ;
			if(connfd > maxfd)
				maxfd = connfd;
			if(i > maxi)
				maxi = i;
			if(--nready == 0)
				continue;
		}
	
		for(i = 0; i <= maxi; i++){
			if((sockfd =client[i]) < 0)
				continue;
			if(FD_ISSET(sockfd, &rset)){
				if((n = recv(sockfd, &msg, sizeof(msg), 0)) == 0){
					close(sockfd);
					FD_CLR(sockfd, &allset);
					client[i] = -1;
				}else{
				
					process_client_request(sockfd, &msg);
				}
				if(--nready == 0)
					break;
			}
		}
	}
	close(sockfd);

	return 0;
}










