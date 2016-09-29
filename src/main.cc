#include "spiderserver.h"

Q_USING_NAMESPACE

class SpiderService : public QService {
	public:
		SpiderService()
		{}

		virtual ~SpiderService()
		{}  

		virtual int32_t init()
		{
			int32_t ret = 0;

			ret = server.init("../conf/init.conf");
			if(ret<0) {
				Q_INFO("SpiderServer: init failed, ret = (%d)!", ret);
				return -1; 
			}
			return 0;
		}

		virtual int32_t run(int32_t argc, char** argv)
		{
			Q_INFO("SpiderServer: now to start...");
			server.start();

			Q_INFO("SpiderServer: now to quit!");
			return 0;
		}

		virtual int32_t destroy()
		{   
			return 0;
		}   

	private:
		SpiderServer server;
};

int32_t main(int32_t argc, char **argv)
{
	SpiderService service;
	return service.main(argc, argv);
}
