#ifndef _SP_INI_READ_WRITE_H_
#define _SP_INI_READ_WRITE_H_

//#include "../Helper/SPDefine.h"
#include "../modules/juce_core/juce_core.h"
#include "map"
#include "vector"

using namespace std;
using namespace juce;

#define TRANSPARENTLEVEL		16
#define PI		3.141592653589793f
#define PI_2	1.570796326794896f
#define DEGREE	PI/180.0

#define min4(x1, x2, x3, x4)	min(min(x1, x2), min(x3, x4))
#define max4(x1, x2, x3, x4)	max(max(x1, x2), max(x3, x4))
#define clamp(value, minValue, maxValue)			((value)<(minValue)?(minValue):((value)>(maxValue)?(maxValue):(value)))

typedef unsigned char byte;
typedef unsigned int  uint;
#define BUF_SIZE 256
typedef std::map<String, String> Key;
typedef std::pair<String, String> pair_Key;

// ��
struct Section
{
	String	m_strSecName;
	Key		m_tKeys;

	Section(const String& app)
	{
		m_strSecName = app;
	}
	String GetSectionName(void)
	{
		return m_strSecName;
	}
};

typedef std::vector<Section*> SectionVec;
typedef std::vector<Section*>::iterator SectionVecIt;

//==============================================================================
/** Ini�ļ��Ķ�д������
*/

class JUCE_API CSPIniReadWrite
{
public:
	CSPIniReadWrite();

	CSPIniReadWrite(const String& strFileName)
	{
		setFileName(strFileName);
	}
	~CSPIniReadWrite();

	/** ���ü�ֵ��bCreate��ָ����������δ����ʱ���Ƿ񴴽� */
	bool setValue(const String& lpSection, const String& lpKey, const String& lpValue,bool bCreate=true); 

	/** ���ü�ֵ��bCreate��ָ����������δ����ʱ���Ƿ񴴽� */
	bool setValue(const String& lpSection, const String& lpKey, int nValue,bool bCreate=true); 

	/** ȡ�ü�ֵ */
	int getInt(const String& lpSection, const String& lpKey, int nDefaultValue = -1);

	/** ȡ�ü�ֵ */
	String getString(const String& lpSection, const String& lpKey, const String& lpDefault = String::empty);

	/** �ж�ָ�����Ƿ���� */
	bool isSectionExist(const String& lpSection);

	/** �ж�key�Ƿ���� */
	bool isKeyExist(const String& lpSection, const String& lpKey);

	/** ȡ�ü�ֵ */
	int getInt(const String& lpSection, int nKey, int nDefaultValue = -1);

	/** ȡ�ü�ֵ */
	String getString(const String& lpSection, int nKey, const String& lpDefault = String::empty);

	/** ɾ��ָ���� */
	bool delKey(const String& lpSection, const String& lpKey); 

	// �޸�ָ���ļ��� */
	bool modifyKey(const String& lpSection, const String& lpKey, const String& lpNewKey); 

	/** ɾ��ָ���� */
	bool delSection(const String& lpSection); 

	/** �����ļ� */
	void update(void);
	
	/** ȡ�ö���Ŀ */
	int getSectionNumber(void);
	/** ȡ��ָ���εļ���Ŀ */
	int getKeyNumber(const String& lpSection);
	/** */
	String findKeyBySectionAndValude(const String& lpSection,const String& lpValue);

	void setFileName(const String& strFileName);  

	/** ����ļ��� */
	const String& getFileName(void);
	/** ȡ�õ�һ���ε����� */
	String getFirstSectionName(void);
	/** ���ڴ������������ļ� */
	bool loadFromBuffer(byte* pBuffer, uint length);

	/** ö�ٳ�ȫ���Ķ��� */
	int getAllSections(vector<String>& arrSection);

	/**��ö�ٳ�һ���ڵ�ȫ��������ֵ */
	int getAllKeysAndValues(vector<String>& arrKey,vector<String>& arrValue,const String& lpSection);

	/** ɾ�����еĶ� */
	void delAllSections(void);

	/** ��������Ϣд���ļ��� */
	void writeFile(const String& pFilePath);
private:
	// �ļ��Ƿ����
	bool isFileExist(const String& strPath);

	// ��ȡ�ļ��е���Ϣ������
	void readFile(void);
	// �ַ���ת��Ϊ����
	int str2Int(const String& strInput);
	// ����ת��Ϊ�ַ���
	String int2Str(int nVal);
	// ȡ��һ���ε�iter
	SectionVecIt getFirstSection(void);
	// ȡ���һ���ε�iter
	SectionVecIt getLastSection(void);
	// ����section
	SectionVecIt findSection(const String& lpSection);

private:
	SectionVec m_ini;
	String m_strPath;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CSPIniReadWrite);
};

#endif