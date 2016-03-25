/********************************************************************************************
**
** Copyright (C) 2010-2014 Terry Niu (Beijing, China)
** Filename:	qzlibmanager.h
** Author:	TERRY-V
** Email:	cnbj8607@163.com
** Support:	http://blog.sina.com.cn/terrynotes
** Date:	2014/08/06
**
*********************************************************************************************/

#ifndef __QZLIBMANAGER_H_
#define __QZLIBMANAGER_H_

#include <zlib.h>
#include "qglobal.h"

Q_BEGIN_NAMESPACE

// QZlibManager字符串压缩类, 使用zlib1g-dev库, 编译时需要加-lz选项
// 如果压缩前字符串较短, 压缩后的长度可能大于原始字符串长度
class QZlibManager {
	public:
		// @函数名: QZlibManager压缩类构造函数
		QZlibManager() :
			max_size_(0),
			now_size_(0),
			byte_buffer_(NULL)
		{}

		// @函数名: QZlibManager压缩类析构函数
		virtual ~QZlibManager()
		{
			if(byte_buffer_!=NULL)
				q_delete_array<uint8_t>(byte_buffer_);
		}

		// @函数名: 初始化函数
		// @参数01: 最大空间
		// @返回值: 成功返回0, 失败返回<0的错误码
		int32_t init(uint64_t max_size)
		{
			max_size_=max_size;
			now_size_=(uint64_t)((max_size_+12)*1.1+1024);
			byte_buffer_=q_new_array<uint8_t>(now_size_);
			if(byte_buffer_==NULL) {
				Q_INFO("QZlibManager: alloc error, size = (%d)!", now_size_);
				return -1;
			}
			return 0;
		}

		// @函数名: 字符串压缩函数
		// @参数01: 压缩前字符串
		// @参数02: 压缩前字符串长度
		// @参数03: 压缩后字符串
		// @参数04: 压缩后字符串的最大空间
		// @返回值: 成功返回压缩后字符串的实际长度, 失败返回<0的错误码
		int32_t compress(uint8_t* src, uint64_t src_len, uint8_t* ptr_out, uint64_t max_out_size)
		{
			if(src==NULL||src_len<=0||ptr_out==NULL||max_out_size<=0)
				return -1;

			uint64_t tempLen=now_size_;
			mutex_.lock();
			int32_t ret=::compress(byte_buffer_, &tempLen, src, src_len);
			if(ret!=Z_OK) {
				mutex_.unlock();
				Q_INFO("QZlibManager::compress error, ret = (%d)!", ret);
				return ret;
			}

			if(max_out_size<tempLen) {
				mutex_.unlock();
				return -2;
			}

			memcpy(ptr_out, byte_buffer_, tempLen);
			mutex_.unlock();
			return tempLen;
		}

		// @函数名: 字符串解压函数
		// @参数01: 压缩字符串
		// @参数02: 压缩字符串长度
		// @参数03: 解压后字符串
		// @参数04: 解压后字符串的最大空间
		// @返回值: 成功返回0, 失败返回<0的错误码
		int32_t uncompress(uint8_t* src, uint64_t src_len, uint8_t* ptr_out, uint64_t max_out_size)
		{
			if(src==NULL||src_len<=0||ptr_out==NULL||max_out_size<=0)
				return -1;

			uint64_t tempLen=now_size_;
			mutex_.lock();
			int32_t ret=::uncompress(byte_buffer_, &tempLen, src, src_len);
			if(ret!=Z_OK) {
				mutex_.unlock();
				Q_INFO("QZlibManager::uncompress error, ret = (%d)!", ret);
				return ret;
			}

			if(max_out_size<tempLen) {
				mutex_.unlock();
				return -2;
			}

			memcpy(ptr_out, byte_buffer_, tempLen);
			mutex_.unlock();
			return tempLen;
		}

		// @函数名: gzip数据压缩函数
		// @参数01: 压缩前字符串
		// @参数02: 压缩前字符串长度
		// @参数03: 压缩后字符串
		// @参数04: 压缩后字符串的最大空间
		// @返回值: 成功返回压缩后字符串的实际长度, 失败返回<0的错误码
		int32_t gzip_compress(uint8_t* src, uint64_t src_len, uint8_t* ptr_out, uint64_t max_out_size)
		{
			if(src==NULL||src_len<=0||ptr_out==NULL||max_out_size<=0)
				return -1;

			z_stream c_stream;
			int32_t err = 0;
			c_stream.zalloc = NULL;
			c_stream.zfree = NULL;
			c_stream.opaque = NULL;

			// 只有设置为MAX_WBITS + 16才能在在压缩文本中带header和trailer
			if(deflateInit2(&c_stream, Z_DEFAULT_COMPRESSION, Z_DEFLATED, MAX_WBITS + 16, 8, Z_DEFAULT_STRATEGY) != Z_OK)
				return -2;

			c_stream.next_in  = src;
			c_stream.avail_in  = src_len;
			c_stream.next_out = ptr_out;
			c_stream.avail_out  = max_out_size;

			while(c_stream.avail_in != 0 && c_stream.total_out < max_out_size)
				if(deflate(&c_stream, Z_NO_FLUSH) != Z_OK)
					return -3;

			if(c_stream.avail_in != 0)
				return c_stream.avail_in;
			for(;;) {
				if((err = deflate(&c_stream, Z_FINISH)) == Z_STREAM_END) break;
				if(err != Z_OK) return -1;
			}

			if(deflateEnd(&c_stream) != Z_OK)
				return -4;
			return c_stream.total_out;
		}

		// @函数名: gzip数据解压函数
		// @参数01: 压缩字符串
		// @参数02: 压缩字符串长度
		// @参数03: 解压后字符串
		// @参数04: 解压后字符串的最大空间
		// @返回值: 成功返回0, 失败返回<0的错误码
		int32_t gzip_uncompress(uint8_t* src, uint64_t src_len, uint8_t* ptr_out, uint64_t max_out_size)
		{
			if(src==NULL||src_len<=0||ptr_out==NULL||max_out_size<=0)
				return -1;

			int32_t err=0;
			z_stream d_stream={0};
			static char dummy_head[2]={0x8 + 0x7 * 0x10, (((0x8 + 0x7 * 0x10) * 0x100 + 30) / 31 * 31) & 0xFF,};
			d_stream.zalloc=NULL;
			d_stream.zfree=NULL;
			d_stream.opaque=NULL;
			d_stream.next_in=src;
			d_stream.avail_in=0;
			d_stream.next_out=ptr_out;

			// 只有设置为MAX_WBITS + 16才能在解压带header和trailer的文本
			if(inflateInit2(&d_stream, MAX_WBITS + 16) != Z_OK)
				return -2;
			if(inflateInit2(&d_stream, 47) != Z_OK)
				return -3;

			while(d_stream.total_out < max_out_size && d_stream.total_in < src_len) {
				d_stream.avail_in = d_stream.avail_out = 1;
				if((err = inflate(&d_stream, Z_NO_FLUSH)) == Z_STREAM_END)
					break;
				if(err != Z_OK) {
					if(err == Z_DATA_ERROR) {
						d_stream.next_in = (Bytef*) dummy_head;
						d_stream.avail_in = sizeof(dummy_head);
						if((err = inflate(&d_stream, Z_NO_FLUSH)) != Z_OK) {
							return -4;
						}
					} else {
						return -5;
					}
				}
			}

			if(inflateEnd(&d_stream) != Z_OK)
				return -6;
			return d_stream.total_out;
		}

	private:
		// 互斥锁
		QMutexLock mutex_;
		// 最大空间
		uint64_t max_size_;
		// 当前空间
		uint64_t now_size_;
		// 内存缓存空间
		uint8_t* byte_buffer_;
};

Q_END_NAMESPACE

#endif // __QZLIBMANAGER_H_
