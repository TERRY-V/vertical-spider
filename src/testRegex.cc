#include "qglobal.h"
#include "qfile.h"
#include "urlprocessor.h"

Q_USING_NAMESPACE

int32_t main(int32_t argc, char** argv)
{
	if(argc!=3) {
		printf("Usage: ./testRegex \"../conf/template.xml\" \"http://star.haibao.com/article/2459836.htm\"\n");
		return -1;
	}

	URLProcessor up;
	int32_t ret=0;

	std::string text = QFile::readAll(argv[1]);
	ret = up.init(text.c_str(), text.length());
	if(ret < 0) {
		printf("Init template (%s), ret = (%d)\n", argv[1], ret);
		return -2;
	}

	char buf[1<<20] = {0};
	int32_t wflag=0;
	ret = up.process(1, q_get_host(std::string(argv[2])).c_str(), argv[2], NULL, buf, sizeof(buf), &wflag);
	if(ret < 0) {
		printf("Process error, ret = (%d)\n", ret);
		return -2;
	} else {
		printf("%.*s\n", ret, buf);
	}

	return 0;
}
