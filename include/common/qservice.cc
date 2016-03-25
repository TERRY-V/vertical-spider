#include "qservice.h"

Q_BEGIN_NAMESPACE

QService::QService()
{}

QService::~QService()
{}

int32_t QService::main(int32_t argc, char** argv)
{
	if(argc<2) {
		fprintf(stderr, ":invalid option\n");
		fprintf(stderr, "Try '--help' for more information!\n");
		return EXIT_FAILURE;
	}

	/*
	 * set locale
	 */
	setlocale(LC_COLLATE, "");

	if(strcmp(argv[1], "-a")==0||strcmp(argv[1], "--author")==0) {
		author();
		return EXIT_SUCCESS;
	} else if(strcmp(argv[1], "-d")==0||strcmp(argv[1], "--daemon")==0) {
		daemonize();			/* run as a daemon */
		createPidFile();		/* create a pid file */

		setproctitle(argc, argv);	/* set process title */

		if(init()) {
			fprintf(stderr, "QService: init error!\n");
			return EXIT_FAILURE;
		}
		if(run(argc, argv)) {
			fprintf(stderr, "QService: run error!\n");
			return EXIT_FAILURE;
		}
		if(destroy()) {
			fprintf(stderr, "QService: run error!\n");
			return EXIT_FAILURE;
		}
		return EXIT_SUCCESS;
	} else if(strcmp(argv[1], "-h")==0||strcmp(argv[1], "--help")==0) {
		help();
		return EXIT_SUCCESS;
	} else if(strcmp(argv[1], "-r")==0||strcmp(argv[1], "--run")==0) {
		setproctitle(argc, argv);	/* set process title */

		if(init()) {
			fprintf(stderr, "QService: init error!\n");
			return EXIT_FAILURE;
		}
		if(run(argc, argv)) {
			fprintf(stderr, "QService: run error!\n");
			return EXIT_FAILURE;
		}
		if(destroy()) {
			fprintf(stderr, "QService: run error!\n");
			return EXIT_FAILURE;
		}
		return EXIT_SUCCESS;
	} else if(strcmp(argv[1], "-v")==0||strcmp(argv[1], "--version")==0) {
		version();
		return EXIT_SUCCESS;
	} else {
		fprintf(stderr, ":invalid option\n");
		fprintf(stderr, "Try '--help' for more information!\n");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

int32_t QService::daemonize()
{
#ifdef WIN32
	return 0;
#else
	int32_t pid;
	int32_t fd;

	/*
	 * Clear file creation mask.
	 */
	umask(0);

	/*
	 * Ignore possible signals.
	 */
	::signal(SIGTTOU, SIG_IGN);
	::signal(SIGTTIN, SIG_IGN);
	::signal(SIGTSTP, SIG_IGN);
	::signal(SIGHUP, SIG_IGN);
	::signal(SIGCHLD, SIG_IGN);

	/*
	 * Become a session leader to lose controlling TTY.
	 */
	if((pid=::fork())) {
		exit(EXIT_SUCCESS);
	} else if(pid<0) {
		perror("fork");
		exit(EXIT_FAILURE);
	}
	::setsid();

	/*
	 * Every output goes to /dev/null.
	 */
	if((fd=open("/dev/null", O_RDWR, 0))!=-1) {
		dup2(fd, STDIN_FILENO);
		dup2(fd, STDOUT_FILENO);
		dup2(fd, STDERR_FILENO);
		if(fd>STDERR_FILENO) close(fd);
	}

	return 0;
#endif
}

void QService::createPidFile(void)
{
	FILE* fp=fopen(SERVICE_PIDFILE, "w");
	if(fp) {
#ifdef WIN32
		fprintf(fp, "%d\n", (int32_t)_getpid());
#else
		fprintf(fp, "%d\n", (int32_t)getpid());
#endif
		fclose(fp);
	}
}

int32_t QService::setproctitle(int32_t argc, char** argv, char* prefix, char* type)
{
	char title[1<<10]={0};

	strcat(title, prefix);
	strcat(title, ":");
	strcat(title, " ");

	strcat(title, type);
	strcat(title, " ");

	for(int32_t i=0; i<argc; ++i)
	{
		strcat(title, argv[i]);
		if(i<argc-1)
			strcat(title, " ");
	}

	strcpy(argv[0], title);
	return 0;
}

void QService::author()
{
	fprintf(stdout, "Author: %s\n", "TERRY-V");
	fprintf(stdout, "E-mail: %s\n", "cnbj8607@163.com");
	fprintf(stdout, "Blog: %s\n", "http://blog.sina.com.cn/terrynotes");
}

void QService::help()
{
	std::string options="Options:\n" \
			     "-a,--author        Show author information...\n" \
			     "-d,--daemon        Run as a daemon...\n" \
			     "-h,--help          Show help message...\n" \
			     "-r,--run           Run this program...\n" \
			     "-v,--version       Show porgram version...\n";

	fprintf(stderr, "Usage:\n%s", options.c_str());
}

void QService::version()
{
	fprintf(stdout, "Version: %s\n", Q_VERSION_STR);
}

Q_END_NAMESPACE
