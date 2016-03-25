#include "downloader.h"

Q_BEGIN_NAMESPACE

Downloader::Downloader() :
	request_buffer_(NULL),
	request_buffer_size_(DOWNLOAD_DEFAULT_SEND_SIZE),
	reply_buffer_(NULL),
	reply_buffer_size_(DOWNLOAD_DEFAULT_RECV_SIZE)
{}

Downloader::~Downloader()
{
	q_delete_array<char>(request_buffer_);
	q_delete_array<char>(reply_buffer_);
}

int32_t Downloader::init()
{
	request_buffer_=q_new_array<char>(request_buffer_size_);
	if(request_buffer_==NULL)
		return DOWNLOAD_ERR_ALLOC;

	reply_buffer_=q_new_array<char>(reply_buffer_size_);
	if(reply_buffer_==NULL)
		return DOWNLOAD_ERR_ALLOC;

	return DOWNLOAD_OK;
}

int32_t Downloader::setServerInfo(ServerInfo& server_info)
{
	strcpy(server_info_.ip, server_info.ip);
	server_info_.port=server_info.port;
	server_info_.timeout=server_info.timeout;
	return DOWNLOAD_OK;
}

int32_t Downloader::process(const char* req_xml, int32_t req_xml_len, char* buf, int32_t buf_size)
{
	if(req_xml==NULL||req_xml_len<=0||buf==NULL||buf_size<=0)
		return DOWNLOAD_ERR;

	/* declare */
	char* ptr_temp=NULL;
	char* p=NULL;

	Q_SOCKET_T sock_client;
	int32_t send_len=0;
	int32_t after_len=0;
	int32_t status=0;
	int32_t rep_xml_len=0;
	int32_t ret=0;

	/* version */
	ptr_temp=request_buffer_;
	*(uint64_t*)ptr_temp=DOWNLOAD_PROTOCOL_VERSION;
	ptr_temp+=sizeof(uint64_t);

	/* after length */
	p=ptr_temp;
	ptr_temp+=sizeof(int32_t);

	/* protocol type */
	*(int16_t*)ptr_temp=DOWNLOAD_PROTOCOL_TYPE;
	ptr_temp+=sizeof(int16_t);

	/* source type */
	*(int16_t*)ptr_temp=DOWNLOAD_SOURCE_TYPE;
	ptr_temp+=sizeof(int16_t);

	/* save param */
	ptr_temp+=14;

	/* command type */
	*(int16_t*)ptr_temp=DOWNLOAD_COMMAND_TYPE;
	ptr_temp+=sizeof(int16_t);

	/* operate type */
	*(int16_t*)ptr_temp=DOWNLOAD_OPERATE_TYPE;
	ptr_temp+=sizeof(int16_t);

	/* xml length */
	*(int32_t*)ptr_temp=req_xml_len;
	ptr_temp+=sizeof(int32_t);

	/* xml data */
	memcpy(ptr_temp, req_xml, req_xml_len);
	ptr_temp+=req_xml_len;

	/* entended length */
	*(int32_t*)ptr_temp=0;
	ptr_temp+=sizeof(int32_t);

	*(int32_t*)p=(ptrdiff_t)(ptr_temp-request_buffer_-12);
	send_len=ptr_temp-request_buffer_;

	/* client --> server */
	ret=q_init_socket();
	if(ret!=0) {
		Q_INFO("TCP socket [%s:%d] init......", server_info_.ip, server_info_.port);
		return DOWNLOAD_ERR_SOCKET_INIT;
	}

	ret=q_connect_socket(sock_client, server_info_.ip, server_info_.port);
	if(ret!=0) {
		q_close_socket(sock_client);
		Q_INFO("TCP socket [%s:%d] connection......", server_info_.ip, server_info_.port);
		return DOWNLOAD_ERR_SOCKET_CONNECT;
	}

	ret=q_set_overtime(sock_client, server_info_.timeout);
	if(ret!=0) {
		q_close_socket(sock_client);
		Q_INFO("TCP socket [%s:%d] set overtime......", server_info_.ip, server_info_.port);
		return DOWNLOAD_ERR_SOCKET_TIMEOUT;
	}

	ret=q_sendbuf(sock_client, request_buffer_, send_len);
	if(ret!=0) {
		q_close_socket(sock_client);
		Q_INFO("TCP socket [%s:%d] send......", server_info_.ip, server_info_.port);
		return DOWNLOAD_ERR_SOCKET_SEND;
	}

	/* server --> client */
	ret=q_recvbuf(sock_client, reply_buffer_, 12);
	if(ret!=0) {
		q_close_socket(sock_client);
		Q_INFO("TCP socket [%s:%d] recv......", server_info_.ip, server_info_.port);
		return DOWNLOAD_ERR_SOCKET_RECV;
	}

	if(*(uint64_t*)reply_buffer_!=DOWNLOAD_PROTOCOL_VERSION) {
		q_close_socket(sock_client);
		Q_INFO("TCP socket [%s:%d] version = (%lu)", server_info_.ip, server_info_.port, *(uint64_t*)reply_buffer_);
		return DOWNLOAD_ERR_PROTOCOL_VERSION;
	}

	after_len=*(int32_t*)(reply_buffer_+sizeof(uint64_t));
	if(after_len<=0||after_len>reply_buffer_size_-12) {
		q_close_socket(sock_client);
		Q_INFO("TCP socket [%s:%d] length = (%d)", server_info_.ip, server_info_.port, after_len);
		return DOWNLOAD_ERR_DATA_LEN;
	}

	ret=q_recvbuf(sock_client, reply_buffer_+12, after_len);
	if(ret!=0) {
		q_close_socket(sock_client);
		Q_INFO("TCP socket [%s:%d] recv body, after_len = (%d)......", server_info_.ip, server_info_.port, after_len);
		return DOWNLOAD_ERR_SOCKET_RECV;
	}

	status=*(int32_t*)(reply_buffer_+26);
	if(status!=0) {
		q_close_socket(sock_client);
		Q_INFO("TCP socket [%s:%d] status = (%d)", server_info_.ip, server_info_.port, status);
		return DOWNLOAD_ERR_STATUS;
	}

	rep_xml_len=*(int32_t*)(reply_buffer_+32);
	if(rep_xml_len<=0) {
		q_close_socket(sock_client);
		Q_INFO("TCP socket [%s:%d] status = (%d)", server_info_.ip, server_info_.port, rep_xml_len);
		return DOWNLOAD_ERR_XML_LEN;
	}

	if(buf+rep_xml_len>buf+buf_size) {
		q_close_socket(sock_client);
		return DOWNLOAD_ERR_BUF_TOO_SMALL;
	}

	memcpy(buf, reply_buffer_+36, rep_xml_len);
	*(buf+rep_xml_len)='\0';

	q_close_socket(sock_client);
	return rep_xml_len;
}

Q_END_NAMESPACE
