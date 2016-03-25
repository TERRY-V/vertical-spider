#include "qdictcreator.h"

Q_BEGIN_NAMESPACE

int32_t QDictCreator::combine(const char* pszInFileName, const char* pszInCustomFileName)
{
	if(pszInFileName==NULL||pszInCustomFileName==NULL)
		return DICT_ERR;

	std::map< std::string, std::vector<std::string> > wordsMap;

	std::vector<std::string> lines;
	std::vector<std::string> common_lines=q_line_tokenize(QFile::readAll(pszInFileName));
	std::vector<std::string> user_lines=q_line_tokenize(QFile::readAll(pszInCustomFileName));

	lines.insert(lines.end(), common_lines.begin(), common_lines.end());
	lines.insert(lines.end(), user_lines.begin(), user_lines.end());

	for(int32_t i=0; i<(int32_t)lines.size(); ++i)
	{
		std::string word=lines[i].substr(1, lines[i].length()-1);
		std::vector<std::string> parts=q_split_any(word, "> ");
		if(parts.size()<2)
			continue;

		std::map< std::string, std::vector<std::string> >::iterator iter=wordsMap.find(parts.at(0));
		if(iter!=wordsMap.end())
		{
			for(int32_t j=1; j<(int32_t)parts.size(); ++j) {
				if(find(iter->second.begin(), iter->second.end(), parts[j])==iter->second.end())
					iter->second.push_back(parts[j]);
			}
		} else {
			std::vector<std::string> semantics(parts.begin()+1, parts.end());
			wordsMap.insert(std::make_pair< std::string, std::vector<std::string> >(parts[0], semantics));
		}
	}

	FILE* fp=fopen("__common.lst", "w");
	Q_CHECK_PTR(fp);

	for(std::map< std::string, std::vector<std::string> >::iterator iter=wordsMap.begin(); \
			iter!=wordsMap.end(); \
			++iter)
	{
		fputc('$', fp);
		fputs(iter->first.c_str(), fp);
		fputs("<<", fp);
		for(int32_t i=0; i<(int32_t)iter->second.size(); ++i) {
			fputs(iter->second.at(i).c_str(), fp);
			if(i!=(int32_t)iter->second.size()-1)
				fputc(' ', fp);
		}
		fputc('\n', fp);
	}

	fclose(fp);

	return DICT_OK;
}

int32_t QDictCreator::pack(const char* pszInFileName, const char* pszOutFileName)
{
	if(pszInFileName==NULL||pszOutFileName==NULL)
		return DICT_ERR;

	dictInfo dict_info;
	dict_info.magicNumber=DICT_MAGIC_NUMBER;
	dict_info.libVersion=DICT_LIB_VERSION;
	dict_info.author=DICT_AUTHOR;
	dict_info.totalNum=0;

	recordHeader record_header;
	record_header.id=0;
	record_header.length=0;

	uint64_t end_mark=DICT_END_MARK;
	int32_t ret=DICT_OK;

	char in_buf[BUFSIZ_1K]={0};
	int32_t in_buf_len=0;

	char out_buf[BUFSIZ_1K]={0};
	int32_t out_buf_len=0;

	FILE *fpR=fopen(pszInFileName, "rb");
	if(fpR==NULL) {
		Q_INFO("Failed in open (%s), error = (%s)", pszInFileName, strerror(errno));
		return DICT_ERR;
	}

	FILE* fpW=fopen(pszOutFileName, "wb");
	if(fpW==NULL) {
		Q_INFO("Failed in open (%s), error = (%s)", pszOutFileName, strerror(errno));
		fclose(fpR);
		return DICT_ERR;
	}

	try {
		if(fwrite(&dict_info, sizeof(dict_info), 1, fpW)!=1)
			throw -1;

		while(fgets(in_buf, sizeof(in_buf), fpR))
		{
			in_buf_len=q_right_trim(in_buf, strlen(in_buf));
			if(in_buf_len<=0) continue;

			char* ptr_word=in_buf+1;
			int32_t len=strstr(ptr_word, "<<")-ptr_word;

			if(strncmp(in_buf, "$", 1)==0) {
				out_buf_len=QTextCodec::utf82unicode(ptr_word, len, out_buf, sizeof(out_buf));
				Q_ASSERT(out_buf_len>=0, "QTextCodec::utf82unicode out_buf_len = (%d)", out_buf_len);

				record_header.id=++dict_info.totalNum;
				record_header.length=out_buf_len-2;

				if(fwrite(&record_header, sizeof(record_header), 1, fpW)!=1)
					throw -2;

				if(fwrite(out_buf+2, out_buf_len-2, 1, fpW)!=1)
					throw -3;

				if(fwrite(&end_mark, sizeof(uint64_t), 1, fpW)!=1)
					throw -4;
			}
		}

		fseek(fpW, sizeof(dictInfo)-4, SEEK_SET);
		if(fwrite(&(dict_info.totalNum), sizeof(dict_info.totalNum), 1, fpW)!=1)
			throw -5;

		Q_INFO("Total: (%d) was packed!", dict_info.totalNum);
	} catch(int32_t err) {
		ret=err;
		Q_INFO("Error: failed in packing, ret = (%d), error = (%s)", ret, strerror(errno));
	}

	if(fpR) fclose(fpR);
	if(fpW) fclose(fpW);

	return ret;
}

int32_t QDictCreator::pack_pos(const char* pszInFileName, const char* pszOutFileName)
{
	if(pszInFileName==NULL||pszOutFileName==NULL)
		return DICT_ERR;

	dictInfo dict_info;
	dict_info.magicNumber=DICT_MAGIC_NUMBER;
	dict_info.libVersion=DICT_LIB_VERSION;
	dict_info.author=DICT_AUTHOR;
	dict_info.totalNum=0;

	recordHeader record_header;
	record_header.id=0;
	record_header.length=0;

	uint64_t end_mark=DICT_END_MARK;
	int32_t ret=DICT_OK;

	char in_buf[BUFSIZ_1K]={0};
	int32_t in_buf_len=0;

	char out_buf[BUFSIZ_1K]={0};
	int32_t out_buf_len=0;

	FILE *fpR=fopen(pszInFileName, "rb");
	if(fpR==NULL) {
		Q_INFO("Failed in open (%s), error = (%s)", pszInFileName, strerror(errno));
		return DICT_ERR;
	}

	FILE* fpW=fopen(pszOutFileName, "wb");
	if(fpW==NULL) {
		Q_INFO("Failed in open (%s), error = (%s)", pszOutFileName, strerror(errno));
		fclose(fpR);
		return DICT_ERR;
	}

	try {
		if(fwrite(&dict_info, sizeof(dict_info), 1, fpW)!=1)
			throw -1;

		while(fgets(in_buf, sizeof(in_buf), fpR))
		{
			in_buf_len=q_right_trim(in_buf, strlen(in_buf));
			if(in_buf_len<=0) continue;

			if(strncmp(in_buf, "$", 1)==0) {
				char* ptr_key=in_buf+1;
				int32_t key_len=strstr(ptr_key, "<<")-ptr_key;

				char* ptr_value=ptr_key+key_len+2;
				int32_t value_len=in_buf_len-key_len-3;

				out_buf_len=QTextCodec::utf82unicode(ptr_key, key_len, out_buf, sizeof(out_buf));
				Q_ASSERT(out_buf_len>=0, "QTextCodec::utf82unicode out_buf_len = (%d)", out_buf_len);

				QMD5 md5;
				record_header.id=md5.MD5Bits64((unsigned char*)(out_buf+2), out_buf_len-2);

				out_buf_len=QTextCodec::utf82unicode(ptr_value, value_len, out_buf, sizeof(out_buf));
				Q_ASSERT(out_buf_len>=0, "QTextCodec::utf82unicode out_buf_len = (%d)", out_buf_len);

				record_header.length=out_buf_len-2;

				if(fwrite(&record_header, sizeof(record_header), 1, fpW)!=1)
					throw -2;

				if(fwrite(out_buf+2, out_buf_len-2, 1, fpW)!=1)
					throw -3;

				if(fwrite(&end_mark, sizeof(uint64_t), 1, fpW)!=1)
					throw -4;

				++dict_info.totalNum;
			}
		}

		fseek(fpW, sizeof(dictInfo)-4, SEEK_SET);
		if(fwrite(&(dict_info.totalNum), sizeof(dict_info.totalNum), 1, fpW)!=1)
			throw -5;

		Q_INFO("Total: (%d) was packed!", dict_info.totalNum);
	} catch(int32_t err) {
		ret=err;
		Q_INFO("Error: failed in packing, ret = (%d), error = (%s)", ret, strerror(errno));
	}

	if(fpR) fclose(fpR);
	if(fpW) fclose(fpW);

	return ret;
}

Q_END_NAMESPACE
