/********************************************************************************************
**
** Copyright (C) 2010-2014 Terry Niu (Beijing, China)
** Filename:	qlogicalparser.h
** Author:	TERRY-V
** Email:	cnbj8607@163.com
** Support:	http://blog.sina.com.cn/terrynotes
** Date:	2013/12/05
**
*********************************************************************************************/

#ifndef __QLOGICALPARSER_H_
#define __QLOGICALPARSER_H_

#include "qglobal.h"

Q_BEGIN_NAMESPACE

// QLogicalParser逻辑解析器
class QLogicalParser: public noncopyable {
	public:
		// @函数名: 构造和析构函数
		QLogicalParser();
		virtual ~QLogicalParser();

		// @函数名: 逻辑解析函数
		// @参数01: 待匹配语法
		// @参数02: 待匹配键
		// @参数03: 带匹配语义项
		// @返回值: 匹配成功返回true，失败返回false
		bool parse(const std::string& gram, const std::string& key, const std::vector<std::string>& meanings);

	private:
		// @函数名: 中缀转后缀表达式
		// @参数01: 待匹配语法
		// @参数02: 待匹配语义项
		// @返回值: 返回void
		void inToPost(const std::string& gram, std::vector<std::string>& v);

		// @函数名: 优先级比较函数
		// @参数01: 优先级字符
		// @返回值: 成功返回优先级, 失败返回<0的错误码
		int32_t priority(const char op);

		// @函数名: 判断是否为操作符
		bool isOperator(const char op);

		// @函数名: 逻辑解析匹配函数
		// @参数01: 待匹配词
		// @参数02: 待匹配语义项
		// @返回值: 匹配成功返回1, 失败返回0
		int32_t calRun(const std::string& key, const std::vector<std::string>& meanings, const std::vector<std::string>& v);

		// @函数名: 操作符运算
		// @参数01: 待匹配词
		// @参数02: 待匹配语义项
		// @参数03: 操作符
		// @返回值: 判断为真返回1, 判断为假返回0
		int32_t doOperator(const std::string& key, const std::vector<std::string>& meanings, const std::string& op);

		// @函数名: 获取单操作符
		bool get1Operand(int32_t& right);

		// @函数名: 获取双操作符
		bool get2Operands(int32_t& left, int32_t& right);

		// @函数名: 清空栈
		void clear();

	protected:
		// 逻辑表达式成员栈
		std::stack<int32_t> s;
};

Q_END_NAMESPACE

#endif // __QLOGICPARSER_H_
