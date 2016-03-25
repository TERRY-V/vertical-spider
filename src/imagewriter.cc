#include "imagewriter.h"

Q_BEGIN_NAMESPACE

ImageWriter::ImageWriter() :
	request_buffer_(NULL),
	request_buffer_size_(WRITER_DEFAULT_SEND_SIZE),
	request_buffer_len_(0),
	reply_buffer_(NULL),
	reply_buffer_size_(WRITER_DEFAULT_RECV_SIZE),
	reply_buffer_len_(0)
{}

ImageWriter::~ImageWriter()
{
	q_delete_array<char>(request_buffer_);
	q_delete_array<char>(reply_buffer_);
}

int32_t ImageWriter::init(const char* ip, uint16_t port, int32_t timeout, int32_t request_buf_size, int32_t reply_buf_size)
{
	if(ip==NULL||request_buf_size<=0||reply_buf_size<=0)
		return WRITER_ERR;

	strcpy(ip_, ip);
	port_=port;

	request_buffer_size_=request_buf_size;
	reply_buffer_size_=reply_buf_size;

	request_buffer_=q_new_array<char>(request_buffer_size_);
	if(request_buffer_==NULL)
		return WRITER_ERR_ALLOC;

	reply_buffer_=q_new_array<char>(reply_buffer_size_);
	if(reply_buffer_==NULL) {
		q_delete_array<char>(request_buffer_);
		return WRITER_ERR_ALLOC;
	}

	return WRITER_OK;
}

int32_t ImageWriter::write(int32_t type, char* ptr_image, int32_t image_len, char* buf, int32_t buf_size)
{
	if(NULL==ptr_image||image_len<=0||NULL==buf||buf_size<=0)
		return WRITER_ERR;

	/* declare */
	char* ptr_temp=NULL;
	char* p=NULL;

	Q_SOCKET_T sock_client=WRITER_INVALID_SOCKET;
	int32_t ret=0;

	int32_t header_len=sizeof(uint64_t)+sizeof(int32_t);
	int32_t after_len=0;

	int32_t status=0;
	int32_t reply_len=0;

	/* version */
	ptr_temp=request_buffer_;
	*(uint64_t*)ptr_temp=WRITER_PROTOCOL_VERSION;
	ptr_temp+=sizeof(uint64_t);

	/* after length */
	p=ptr_temp;
	ptr_temp+=sizeof(int32_t);

	/* protocol type */
	*(int16_t*)ptr_temp=WRITER_PROTOCOL_TYPE;
	ptr_temp+=sizeof(int16_t);

	/* source type */
	*(int16_t*)ptr_temp=WRITER_SOURCE_TYPE;
	ptr_temp+=sizeof(int16_t);

	/* save param */
	ptr_temp+=14;

	/* command type */
	*(int16_t*)ptr_temp=WRITER_COMMAND_TYPE;
	ptr_temp+=sizeof(int16_t);

	/* operate type */
	*(int16_t*)ptr_temp=type;
	ptr_temp+=sizeof(int16_t);

	/* image length */
	*(int32_t*)ptr_temp=image_len;
	ptr_temp+=sizeof(int32_t);

	/* image data */
	memcpy(ptr_temp, ptr_image, image_len);
	ptr_temp+=image_len;

	/* entended length */
	*(int32_t*)ptr_temp=0;
	ptr_temp+=sizeof(int32_t);

	*(int32_t*)p=(ptrdiff_t)(ptr_temp-request_buffer_-header_len);
	request_buffer_len_=(ptrdiff_t)(ptr_temp-request_buffer_);

	/* client --> server */
	ret=q_init_socket();
	if(ret!=0) {
		q_close_socket(sock_client);
		Q_INFO("TCP socket [%s:%d] init......", ip_, port_);
		return WRITER_ERR_SOCKET_INIT;
	}

	ret=q_connect_socket(sock_client, ip_, port_);
	if(ret!=0) {
		q_close_socket(sock_client);
		Q_INFO("TCP socket [%s:%d] connection......", ip_, port_);
		return WRITER_ERR_SOCKET_CONNECT;
	}

	ret=q_set_overtime(sock_client, timeout_);
	if(ret!=0) {
		q_close_socket(sock_client);
		Q_INFO("TCP socket [%s:%d] set overtime......", ip_, port_);
		return WRITER_ERR_SOCKET_TIMEOUT;
	}

	ret=q_sendbuf(sock_client, request_buffer_, request_buffer_len_);
	if(ret!=0) {
		q_close_socket(sock_client);
		Q_INFO("TCP socket [%s:%d] send......", ip_, port_);
		return WRITER_ERR_SOCKET_SEND;
	}

	/* server --> client */
	ret=q_recvbuf(sock_client, reply_buffer_, header_len);
	if(ret!=0) {
		q_close_socket(sock_client);
		Q_INFO("TCP socket [%s:%d] recv......", ip_, port_);
		return WRITER_ERR_SOCKET_RECV;
	}

	if(*(uint64_t*)reply_buffer_!=WRITER_PROTOCOL_VERSION) {
		q_close_socket(sock_client);
		Q_INFO("TCP socket [%s:%d] version = (%lu)", ip_, port_, *(uint64_t*)reply_buffer_);
		return WRITER_ERR_PROTOCOL_VERSION;
	}

	after_len=*(int32_t*)(reply_buffer_+sizeof(uint64_t));
	if(after_len<=0||reply_buffer_+header_len+after_len>reply_buffer_+reply_buffer_size_) {
		q_close_socket(sock_client);
		Q_INFO("TCP socket [%s:%d] length = (%d)", ip_, port_, after_len);
		return WRITER_ERR_DATA_LEN;
	}

	ret=q_recvbuf(sock_client, reply_buffer_+header_len, after_len);
	if(ret!=0) {
		q_close_socket(sock_client);
		Q_INFO("TCP socket [%s:%d] recv body, after_len = (%d)......", ip_, port_, after_len);
		return WRITER_ERR_SOCKET_RECV;
	}

	status=*(int32_t*)(reply_buffer_+26);
	if(status!=0) {
		q_close_socket(sock_client);
		Q_INFO("TCP socket [%s:%d] status = (%d)", ip_, port_, status);
		return WRITER_ERR_STATUS;
	}

	reply_len=*(int32_t*)(reply_buffer_+32);
	if(reply_len<=0) {
		q_close_socket(sock_client);
		Q_INFO("TCP socket [%s:%d] status = (%d)", ip_, port_, reply_len);
		return WRITER_ERR_XML_LEN;
	}

	if(buf+reply_len>buf+buf_size) {
		q_close_socket(sock_client);
		return WRITER_ERR_BUF_TOO_SMALL;
	}

	memcpy(buf, reply_buffer_+36, reply_len);
	*(buf+reply_len)='\0';

	q_close_socket(sock_client);
	return reply_len;
}

Q_END_NAMESPACE
