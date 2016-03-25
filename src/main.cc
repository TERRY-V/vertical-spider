#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <cassert>

#include "qspiderserver.h"

class QSpiderService : public QService {
	public:
		QSpiderService()
		{}

		virtual ~QSpiderService()
		{}

		virtual int32_t init()
		{
			int32_t ret=qss.init("../conf/init.cnf");
			if(ret<0) {
				Q_INFO("QSpiderServer: init failed, ret = (%d)!", ret);
				return -1;
			}
			return 0;
		}

		virtual int32_t run(int32_t argc, char** argv)
		{
			Q_INFO("QSpiderServer: init success, now to start...");
			qss.run();
			Q_INFO("QSpiderServer now to quit!");
			return 0;
		}

		virtual int32_t destroy()
		{
			return 0;
		}
	
	private:
		QSpiderServer qss;
};

int32_t main(int32_t argc, char **argv)
{
	QSpiderService qss;
	return qss.main(argc, argv);
}
