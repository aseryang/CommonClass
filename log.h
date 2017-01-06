#pragma once
#include <log4cpp/Category.hh>
#include <log4cpp/Appender.hh>
#include <log4cpp/FileAppender.hh>
#include <log4cpp/Priority.hh>
#include <log4cpp/PatternLayout.hh>
#include <log4cpp/RollingFileAppender.hh>
#define LogInfo(str) {MyLog::getInstance()->logInfo(str);}
#define LogWarn(str) {MyLog::getInstance()->logWarn(str);}
#define LogError(str) {MyLog::getInstance()->logError(str);}
class MyLog
{
private:
		MyLog()
		{
			log4cpp::PatternLayout* pLayOut = new log4cpp::PatternLayout;
			pLayOut->setConversionPattern("%d: %p %c%x: %m%n");
			log4cpp::Appender* fileAppender = new log4cpp::FileAppender("fileAppender", "yy.log");
			fileAppender->setLayout(pLayOut);
			log4cpp::Category& root = log4cpp::Category::getRoot().getInstance("RootName");
			root.addAppender(fileAppender);
			root.setPriority(log4cpp::Priority::DEBUG);
			pRoot = &root;
		}
public:
	static MyLog* getInstance()
	{
		if (nullptr == pInstance)
		{
			pInstance = new MyLog;
		}
		return pInstance;
	}
	void logInfo(std::string str)
	{
		pRoot->info(str);
	}
	void logWarn(std::string str)
	{
		pRoot->warn(str);
	}
	void logError(std::string str)
	{
		pRoot->error(str);
	}
protected:
private:
	static MyLog* pInstance;
	log4cpp::Category* pRoot;
};
