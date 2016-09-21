#include "SPIniReadWrite.h"
//#include "../Helper/SPDelete.h"
#include "wchar.h"

#if JUCE_WINDOWS
#include "windows.h"
#endif

static String compositeSectionString(const String& strInput);
static String compositeKeyString(const String& strKey, const String& strValue);

static void Utf8ToUnicode(const char* szUtf8, juce_wchar* szUnicode)
{
	int i = 0;
	int j = 0;
	while(szUtf8[i] != 0)
	{
		if ((szUtf8[i] & 0x80) == 0x00)//һλ
		{
			szUnicode[j] = szUtf8[i];
			i += 1;
		}
		else if ((szUtf8[i] & 0xE0) == 0xC0)//��λ
		{
			szUnicode[j] = ((szUtf8[i] & 0x1F) << 6) | (szUtf8[i + 1] & 0x3F);
			i += 2;
		}
		else if ((szUtf8[i] & 0xF0) == 0xE0)
		{
			szUnicode[j] = ((szUtf8[i] & 0x0F) << 12) | ((szUtf8[i + 1] & 0x3F) << 6) | 
				(szUtf8[i + 2] & 0x3F);
			i += 3;
		}
		else if ((szUtf8[i] & 0xF8) == 0xF0)
		{
			szUnicode[j] = ((szUtf8[i] & 0x07) << 18) | ((szUtf8[i + 1] & 0x3F) << 12) | 
				((szUtf8[i + 2] & 0x3F) << 6) | (szUtf8[i + 3] & 0x3F);
			i += 4;
		}
		else if ((szUtf8[i] & 0xFC) == 0xF8)
		{
			szUnicode[j] = ((szUtf8[i] & 0x03) << 24) | ((szUtf8[i + 1] & 0x3F) << 18) | 
				((szUtf8[i + 2] & 0x3F) << 12) | ((szUtf8[i + 3] & 0x3F) << 6) | (szUtf8[i + 4] & 0x3F);
			i += 5;
		}
		else if ((szUtf8[i] & 0xFE) == 0xFC)
		{
			szUnicode[j] = ((szUtf8[i] & 0x01) << 30) | ((szUtf8[i + 1] & 0x3F) << 18) |
				((szUtf8[i + 2] & 0x3F) << 18) | ((szUtf8[i + 3] & 0x3F) << 12) | 
				((szUtf8[i + 4] & 0x3F) << 6) | (szUtf8[i + 5] & 0x3F);
			i += 6;
		}
		else
		{
			break;
		}
		j += 1;
	}
	szUnicode[j] = 0;
}

CSPIniReadWrite::CSPIniReadWrite()
{

}


CSPIniReadWrite::~CSPIniReadWrite()
{
    delAllSections();
}

/*==============================================================================
Function Name:  GetFirstSection
Summary      :  ȡ�õ�һ���ε�iter
Input        :
Output       :
Return value :  �ɹ�orʧ��
==============================================================================*/
SectionVecIt CSPIniReadWrite::getFirstSection(void)
{
    return m_ini.begin();
}

/*==============================================================================
Function Name:  GetLastSection
Summary      :  ȡ�����һ���ε�iter
Input        :
Output       :
Return value :  �ɹ�orʧ��
==============================================================================*/
SectionVecIt CSPIniReadWrite::getLastSection(void)
{
    return m_ini.end();
}

/*==============================================================================
Function Name:  CompositeSectionString
Summary      :  ��ָ�����ַ�����ǰ�������"["��"]"
Input        :
Output       :
Return value :
==============================================================================*/
String compositeSectionString(const String& strInput)
{
	String strTmp = "[";
	strTmp += strInput;
	strTmp += "]";
	return strTmp;
}

/*==============================================================================
Function Name:  CompositeKeyString
Summary      :  ��=����key��value
Input        :
Output       :
Return value :
==============================================================================*/
String compositeKeyString(const String& strKey,const String& strValue)
{
	return strKey + "=" + strValue;
}
/*==============================================================================
Function Name:  WriteFile
Summary      :  �����ļ�
Input        :
Output       :
Return value :
==============================================================================*/
void CSPIniReadWrite::writeFile(const String& pFilePath)
{
	File file(pFilePath);
	if (file.exists() == false)
	{
		file.create();
	}
	ScopedPointer<FileOutputStream> pOutputStream(file.createOutputStream());
	if (pOutputStream == nullptr)
	{
		return;
	}
	pOutputStream->setPosition(0);
	pOutputStream->truncate();
	//д��Unicode��ʽ�ļ�
	pOutputStream->writeByte(0xFF);
	pOutputStream->writeByte(0xFE);

    SectionVecIt iPG = getFirstSection();

    for (; iPG != getLastSection(); iPG++)
    {
		// д��
		pOutputStream->writeText(compositeSectionString((*iPG)->GetSectionName()), true, false);
		pOutputStream->writeText(pOutputStream->getNewLineString(), true, false);

		// д����ֵ
        Key::iterator iKey = (*iPG)->m_tKeys.begin();
        for (; iKey != (*iPG)->m_tKeys.end(); iKey++)
        {
			pOutputStream->writeText(compositeKeyString(iKey->first, iKey->second), true, false);
			pOutputStream->writeText(pOutputStream->getNewLineString(), true, false);
        }
    }
	pOutputStream->flush();
}

/*==============================================================================
Function Name:  SetValue
Summary      :  ���ü�ֵ��bCreate��ָ����������δ����ʱ���Ƿ񴴽���
Input        :
                @lpSection  ����
                @lpKey      ����
                @lpValue    ������ֵ
                @bCreate    ��������ʱ�Ƿ񴴽�
Output       :
Return value :  �ɹ�orʧ��
==============================================================================*/
bool CSPIniReadWrite::setValue(const String& lpSection, const String& lpKey, const String& lpValue, bool bCreate)
{
	SectionVecIt iPG = findSection(lpSection);
	if (iPG == getLastSection())
	{
		if (bCreate)
		{
			// �β����ڣ�ȫ������
			m_ini.push_back(new Section(lpSection));
			m_ini.back()->m_tKeys.insert(pair_Key(lpKey, lpValue));
		}
		return false;
	}

	Key::iterator iKEY = (*iPG)->m_tKeys.find(lpKey);
	if (iKEY == (*iPG)->m_tKeys.end())
	{
		// �������ڣ�������
		if (bCreate)
		{
			// ������
			(*iPG)->m_tKeys.insert(pair_Key(lpKey, lpValue));
		}
		return false;
	}
	else
	{
		// �����ڣ��޸ļ�ֵ
		iKEY->second = lpValue;
		return true;
	}
}

/*==============================================================================
Function Name:  SetValue
Summary      :  ���ü�ֵ��bCreate��ָ����������δ����ʱ���Ƿ񴴽���
Input        :
                @lpSection  ����
                @lpKey      ����
                @lpValue    ������ֵ
                @bCreate    ��������ʱ�Ƿ񴴽�
Output       :
Return value :  �ɹ�orʧ��
==============================================================================*/
bool CSPIniReadWrite::setValue(const String& lpSection, const String& lpKey, int nValue, bool bCreate)
{
    return setValue(lpSection, lpKey, String(nValue), bCreate);
}

/*==============================================================================
Function Name:  GetFirstSectionName
Summary      :  ȡ�õ�һ���ε�iter
Input        :
Output       :
Return value :  ����
==============================================================================*/
String CSPIniReadWrite::getFirstSectionName(void)
{
    if (m_ini.size() == 0)
    {
        return "";
    }

    return (*getFirstSection())->GetSectionName();
}

/*==============================================================================
Function Name:  Update
Summary      :  �����ļ�
Input        :
Output       :
Return value :
==============================================================================*/
void CSPIniReadWrite::update(void)
{
	// ��д����ʱ�ļ����ٸ���ԭ�ļ�
	File file(m_strPath);
	juce::Random r1;
	String strTempFile = m_strPath + String(r1.nextInt());
	writeFile(strTempFile);
	File(strTempFile).moveFileTo(file);
}

/*==============================================================================
Function Name:  GetInt
Summary      :  ȡ�ü�ֵ
Input        :
                @lpKey      ��
                @lpSection  ��
Output       :
Return value :  ��ֵ��-1��ʾʧ�ܣ�
==============================================================================*/
int CSPIniReadWrite::getInt(const String& lpSection, const String& lpKey, int nDefaultValue)
{
	SectionVecIt iPG = findSection(lpSection);
	if (iPG == getLastSection())
	{
		return nDefaultValue;
	}

	Key::iterator iKEY = (*iPG)->m_tKeys.find(lpKey);
	if (iKEY == (*iPG)->m_tKeys.end())
	{
		return nDefaultValue;
	}
	return str2Int(iKEY->second);
}

/*==============================================================================
Function Name:  GetString
Summary      :  ȡ�ü�ֵ
Input        :
                @lpKey      ��
                @lpSection  ��
Output       :
Return value :  ��ֵ��""��ʾʧ�ܣ�
==============================================================================*/
String CSPIniReadWrite::getString(const String& lpSection, const String& lpKey, const String& lpDefault)
{
	SectionVecIt iPG = findSection(lpSection);
	if (iPG == getLastSection())
	{
		return lpDefault;
	}

	Key::iterator iKEY = (*iPG)->m_tKeys.find(lpKey);
	if (iKEY == (*iPG)->m_tKeys.end())
	{
		return lpDefault;
	}
	return iKEY->second;
}

/*==============================================================================
Function Name:  GetInt
Summary      :  ȡ�ü�ֵ
Input        :
                @nKey       ��
                @lpSection  ��
Output       :
Return value :  ��ֵ��-1��ʾʧ�ܣ�
==============================================================================*/
int CSPIniReadWrite::getInt(const String& lpSection, int nKey, int nDefaultValue)
{
    return getInt(lpSection, String(nKey), nDefaultValue);
}

/*==============================================================================
Function Name:  GetString
Summary      :  ȡ�ü�ֵ
Input        :
                @nKey       ��
                @lpSection  ��
Output       :
Return value :  ��ֵ��""��ʾʧ�ܣ�
==============================================================================*/
String CSPIniReadWrite::getString(const String& lpSection, int nKey, const String& lpDefault)
{
    return getString(lpSection, String(nKey), lpDefault);
}

bool CSPIniReadWrite::isSectionExist(const String& lpSection)
{
	SectionVecIt iPG = findSection(lpSection);
	if (iPG == getLastSection())
	{
		return false;
	}
	return true;
}
/*==============================================================================
Function Name:  IsKeyExist
Summary      :  �ж�key�Ƿ����
Input        :
Output       :
Return value :  �ε���Ŀ
==============================================================================*/
bool CSPIniReadWrite::isKeyExist(const String& lpSection, const String& lpKey)
{
	SectionVecIt iPG = findSection(lpSection);
	if (iPG == getLastSection())
	{
		return false;
	}

	Key::iterator iKEY = (*iPG)->m_tKeys.find(lpKey);
	if (iKEY == (*iPG)->m_tKeys.end())
	{
		return false;
	}
	return true;
}

/*==============================================================================
Function Name:  GetSectionNumber
Summary      :  �ε���Ŀ
Input        :
Output       :
Return value :  �ε���Ŀ
==============================================================================*/
int CSPIniReadWrite::getSectionNumber(void)
{
    return m_ini.size();
}

/*==============================================================================
Function Name:  GetKeyNumber
Summary      :  ȡ��ָ���εļ�����Ŀ
Input        :
                @lpSection ��
Output       :
Return value :  ������Ŀ
==============================================================================*/
int CSPIniReadWrite::getKeyNumber(const String& lpSection)
{
	SectionVecIt iPG = findSection(lpSection);
	if (iPG == getLastSection())
	{
		return 0;
	}
	// �ҵ���
     return (*iPG)->m_tKeys.size();
}

/*==============================================================================
Function Name:  Str2Int
Summary      :  �ַ���ת��Ϊ����
Input        :
                @a_strInput �ַ���
Output       :
Return value :  ����
==============================================================================*/
int CSPIniReadWrite::str2Int(const String& str)
{
    return str.getIntValue();
}
//int to string
/*==============================================================================
Function Name:  Int2Str
Summary      :  ����ת��Ϊ�ַ���
Input        :
                @a_nVal ����
Output       :
Return value :  �ַ���
==============================================================================*/
String CSPIniReadWrite::int2Str(int nVal)
{
    return String(nVal);
}

/*==============================================================================
Function Name:  SetFileName
Summary      :  �����ļ�·��
Input        :
                @strFileName �ļ�·��
Output       :
Return value :
==============================================================================*/
void CSPIniReadWrite::setFileName(const String& strFileName)
{
	m_strPath = strFileName;
	if (isFileExist(strFileName))
	{
		delAllSections();
		readFile();
	}
	else
	{
		//LogError(L"IniReadWrite", "%s is not existed!", lpFileName);
	}
}

bool CSPIniReadWrite::loadFromBuffer(byte* pBuffer, uint length)
{
	if (pBuffer)
	{
		delAllSections();

		uint i = 0;
		uint start = 0;
		uint end = 0;

		String strOneLine;				// һ���ı�����
		String strKey;					// ����
		String strValue;				// ��ֵ
		Section *myPG = nullptr;		// �ε�iter
		int squarebrackets_left = -1;	// �������ŵ�λ��
		int squarebrackets_right = - 1;	// �������ŵ�λ��
		int equalsymbol = -1;			// �Ⱥŵ�λ��

		//unicode16
		if (length >= 2 && pBuffer[0] == 0xFF && pBuffer[1] == 0xFE)
		{
			CharPointer_UTF16::CharType byTemp;
			CharPointer_UTF16::CharType* pTBuffer = (CharPointer_UTF16::CharType*)(pBuffer + 2);
			length = (length - 2)/2;
// 			i = 2;
// 			start = i;
			while (1)
			{
				byTemp = pTBuffer[i];
				if (byTemp != 0 && byTemp != L'\r' && byTemp != L'\n' && i < length)
				{
					i++;
				}
				else
				{
					end = i;
					if (start == end)
					{
						if (byTemp == 0 || i >= length)
						{
							break;
						}
						else
						{
							start = end + 1;
							i = start;
							continue;
						}
					}
					//����һ������
					strOneLine = String(CharPointer_UTF16(pTBuffer + start), CharPointer_UTF16(pTBuffer + end)).trim();

					// �ж��Ƿ��ǿ���
					if (strOneLine.isEmpty())
					{
						if (byTemp == 0 || i >= length)
						{
							break;
						}
						else
						{
							start = end + 1;
							i = start;
							continue;
						}
					}

					squarebrackets_left = strOneLine.indexOfChar(L'[');
					squarebrackets_right = strOneLine.indexOfChar(L']');

					// ���´���
					if ((squarebrackets_left != -1) && (squarebrackets_right != -1) && (squarebrackets_left < squarebrackets_right))
					{
						int equalPos = strOneLine.indexOfChar(L'=');
						if (equalPos == -1 || (equalPos > squarebrackets_left && equalPos < squarebrackets_right))
						{
							strKey = strOneLine.substring(squarebrackets_left + 1, squarebrackets_right);

							myPG = new Section(strKey);
							m_ini.push_back(myPG);

							if (byTemp == 0 || i >= length)
							{
								break;
							}
							else
							{
								start = end + 1;
								i = start;
								continue;
							}
						}
					}

					// ���´���key
					equalsymbol = strOneLine.indexOfChar(L'=');
					strKey = strOneLine.substring(0, equalsymbol).trim();	// ȡ�ü���

					// ȡ�ü�ֵ
					strValue = strOneLine.substring(equalsymbol + 1, strOneLine.length()).trim();

					m_ini.back()->m_tKeys.insert(pair_Key(strKey, strValue));


					if (byTemp == 0 || i >= length)
					{
						break;
					}
					start = end + 1;
					i = start;
				}
			}
		}
		//UTF8
		else if (length >= 3 && pBuffer[0] == 0xEF && pBuffer[1] == 0xBB && pBuffer[2] == 0xBF)
		{
			i = 3;// Ҫ�ȼ�ȥ��־λ
			start = i;
			CharPointer_UTF8::CharType byTemp;

			while (1)
			{
				byTemp = pBuffer[i];
				if (byTemp != 0 && byTemp != '\r' && byTemp != '\n' && i < length)
				{
					i++;
				}
				else
				{
					end = i;
					if (start == end)
					{
						if (byTemp == 0 || i >= length)
						{
							break;
						}
						else
						{
							start = end + 1;
							i = start;
							continue;
						}
					}
					//����һ������
					strOneLine = String(CharPointer_UTF8((CharPointer_UTF8::CharType*)(pBuffer + start)),
						CharPointer_UTF8((CharPointer_UTF8::CharType*)(pBuffer + end))).trim();

					// �ж��Ƿ��ǿ���
					if (strOneLine.isEmpty())
					{
						if (byTemp == 0 || i >= length)
						{
							break;
						}
						else
						{
							start = end + 1;
							i = start;
							continue;
						}
					}

					squarebrackets_left = strOneLine.indexOfChar(L'[');
					squarebrackets_right = strOneLine.indexOfChar(L']');

					// ���´���
					if ((squarebrackets_left != -1) && (squarebrackets_right != -1) && (squarebrackets_left < squarebrackets_right))
					{
						int equalPos = strOneLine.indexOfChar(L'=');
						if (equalPos == -1 || (equalPos > squarebrackets_left && equalPos < squarebrackets_right))
						{
							strKey = strOneLine.substring(squarebrackets_left + 1, squarebrackets_right);
							myPG = new Section(strKey);
							m_ini.push_back(myPG);

							if (byTemp == 0 || i >= length)
							{
								break;
							}
							else
							{
								start = end + 1;
								i = start;
								continue;
							}
						}
					}

					// ���´���key
					equalsymbol = strOneLine.indexOfChar(L'=');

					strKey = strOneLine.substring(0, equalsymbol).trim();	// ȡ�ü���

					// ȡ�ü�ֵ
					strValue = strOneLine.substring(equalsymbol + 1, strOneLine.length()).trim();

					m_ini.back()->m_tKeys.insert(pair_Key(strKey, strValue));

					if (byTemp == 0 || i >= length)
					{
						break;
					}
					start = end + 1;
					i = start;
				}
			}
		}
		else
		{
			CharPointer_ASCII::CharType byTemp;

			while (1)
			{
				byTemp = pBuffer[i];
				if (byTemp != 0 && byTemp != '\r' && byTemp != '\n' && i < length)
				{
					i++;
				}
				else
				{
					end = i;
					if (start == end)
					{
						if (byTemp == 0 || i >= length)
						{
							break;
						}
						else
						{
							start = end + 1;
							i = start;
							continue;
						}
					}
					//����һ������
					strOneLine = String((CharPointer_ASCII::CharType*)(pBuffer + start), end - start).trim();

					// �ж��Ƿ��ǿ���
					if (strOneLine.isEmpty())
					{
						if (byTemp == 0 || i >= length)
						{
							break;
						}
						else
						{
							start = end + 1;
							i = start;
							continue;
						}
					}

					squarebrackets_left = strOneLine.indexOfChar(L'[');
					squarebrackets_right = strOneLine.indexOfChar(L']');

					// ���´���
					if ((squarebrackets_left != -1) && (squarebrackets_right != -1) && (squarebrackets_left < squarebrackets_right))
					{
						int equalPos = strOneLine.indexOfChar(L'=');
						if (equalPos == -1 || (equalPos > squarebrackets_left && equalPos < squarebrackets_right))
						{
							strKey = strOneLine.substring(squarebrackets_left + 1, squarebrackets_right);
							myPG = new Section(strKey);
							m_ini.push_back(myPG);

							if (byTemp == 0 || i >= length)
							{
								break;
							}
							else
							{
								start = end + 1;
								i = start;
								continue;
							}
						}
					}

					// ���´���key
					equalsymbol = strOneLine.indexOfChar(L'=');
					if (equalsymbol != -1)
					{
						strKey = strOneLine.substring(0, equalsymbol).trim();	// ȡ�ü���

						// ȡ�ü�ֵ
						strValue = strOneLine.substring(equalsymbol + 1, strOneLine.length()).trim();

						m_ini.back()->m_tKeys.insert(pair_Key(strKey, strValue));
					}

					if (byTemp == 0 || i >= length)
					{
						break;
					}
					start = end + 1;
					i = start;
				}
			}
		}

		return true; 
	}
	return false;
}

/*==============================================================================
Function Name:  ReadFile
Summary      :  ��ȡini�ļ�������
Input        :
Output       :
Return value :
==============================================================================*/

void CSPIniReadWrite::readFile(void)
{
	File file(m_strPath);
	if (file.existsAsFile())
	{
		ScopedPointer<FileInputStream> pInputStream(file.createInputStream());
		if (pInputStream)
		{
			size_t nSize = static_cast<size_t>(file.getSize());
			MemoryBlock memoryBlock(nSize + 1, true);
			if (pInputStream->read(memoryBlock.getData(), nSize) == nSize)
			{
				loadFromBuffer((byte*)memoryBlock.getData(), nSize);
			}
		}
	}
}

/*==============================================================================
Function Name:  GetFileName
Summary      :  ȡ��ini�ļ�·��
Input        :
Output       :
Return value :  ini�ļ�·��
==============================================================================*/
const String& CSPIniReadWrite::getFileName(void)
{
    return m_strPath;
}

/*==============================================================================
Function Name:  DelSection
Summary      :  ɾ��section��
Input        :
                @lpSection  �������е�����
Output       :
Return value :  �ɹ�orʧ��
==============================================================================*/
bool CSPIniReadWrite::delSection(const String& lpSection)  //ɾ������
{
	SectionVecIt iPG = findSection(lpSection);
	if (iPG == getLastSection())
	{
		return false;
	}

    // �ҵ���
    //SAFE_DELETE(*iPG);
    m_ini.erase(iPG);
    return true;
}

/*==============================================================================
Function Name:  DelKey
Summary      :  ɾ���ֶ�
Input        :
                @lpSection  �������е�����
                @LPCTSTR lpKey,     ��=��ߵ��ֶ���
Output       :
Return value :  �ɹ�orʧ��
==============================================================================*/
bool CSPIniReadWrite::delKey(const String& lpSection, const String& lpKey)
{
	SectionVecIt iPG = findSection(lpSection);
	if (iPG == getLastSection())
	{
		return false;
	}
	Key::iterator iKEY = (*iPG)->m_tKeys.find(lpKey);
	if (iKEY == (*iPG)->m_tKeys.end())
	{
		return false;
	}
	// �ҵ���
	(*iPG)->m_tKeys.erase(iKEY);
	return true;
}

/*==============================================================================
Function Name:  ModifyKey
Summary      :  �޸�ָ���ļ���
Input		 :
Output       :
Return value :   
==============================================================================*/
bool CSPIniReadWrite::modifyKey(const String& lpSection, const String& lpKey, const String& lpNewKey)
{
	SectionVecIt iPG = findSection(lpSection);
	if (iPG == getLastSection())
	{
		return false;
	}
	
	Key::iterator iKEY = (*iPG)->m_tKeys.find(lpKey);
	if (iKEY == (*iPG)->m_tKeys.end())
	{
		return false;
	}
	
	// �ҵ���,��ɾ���ü���Ȼ������½�
	// ����ֵ
	(*iPG)->m_tKeys.insert(make_pair(lpKey, lpNewKey));
	return true;
}

/*==============================================================================
Function Name:  GetAllSections
Summary      :  ȡ�����е�section��
Input		 :
Output       :
				@arrSection���еĶ�������
Return value :    ���еĶεĸ���
==============================================================================*/
int CSPIniReadWrite::getAllSections(vector<String>& arrSection)
{
	SectionVecIt iPG = getFirstSection();

	for (; iPG != getLastSection(); iPG++)
	{
		arrSection.push_back((*iPG)->GetSectionName());
	}
	return arrSection.size();
}

/*==============================================================================
Function Name:  GetAllKeysAndValues
Summary      :  ȡ���ƶ���section�ε����е�key�Լ�key��Ӧ��ֵ
Input		 :
				@lpSectionָ����section��
Output       :
				@arrKey���е�=��ߵ�key�ļ���
				@arrValue���е�=�ұߵ�key��ֵ�ļ���
Return value :    key�ĸ���
==============================================================================*/
int CSPIniReadWrite::getAllKeysAndValues(vector<String>& arrKey, vector<String>& arrValue, const String& lpSection)
{
	SectionVecIt iPG = findSection(lpSection);
	if (iPG == getLastSection())
	{
		return -1;
	}

	// �ҵ��˶�,ȡ�øö����еļ���ֵ
	Key::iterator iKEY = (*iPG)->m_tKeys.begin();

	for (; iKEY != (*iPG)->m_tKeys.end(); iKEY++)
	{
		arrKey.push_back(iKEY->first);
		arrValue.push_back(iKEY->second);
	}
	return arrKey.size();
}


/*==============================================================================
Function Name:  DelAllSections
Summary      :  ɾ�����еĶ�
Input        :
Output       :
Return value :
==============================================================================*/
void CSPIniReadWrite::delAllSections()
{
	//DelObj(m_ini);
}

/*==============================================================================
Function Name:  FindSection
Summary      :  ����ָ���Ķ�
Input        :
Output       :
Return value :
==============================================================================*/
SectionVecIt CSPIniReadWrite::findSection(const String& lpSection)
{
	SectionVecIt iPG = getFirstSection();

	for (; iPG != getLastSection(); iPG++)
	{
		if ((*iPG)->GetSectionName() == lpSection)
		{
			break;
		}
	}
	return iPG; // δ�ҵ���ʱ�򷵻ص���GetLastSection;
}

/*==============================================================================
Function Name:  FindKeyBySectionAndValude
Summary      :  ͨ��ָ���Ķκ�ĳ������ֵ�����Ҹü�
Input        :
Output       :
Return value :
==============================================================================*/
String CSPIniReadWrite::findKeyBySectionAndValude(const String& lpSection, const String& lpValue)
{
	SectionVecIt iPG = findSection(lpSection);
	if (iPG == getLastSection())
	{
		return "";
	}

	Key::iterator iKEY = (*iPG)->m_tKeys.begin();

	for (; iKEY != (*iPG)->m_tKeys.end(); iKEY++)
	{
		if (iKEY->second != lpValue)
		{
			break;
		}
	}
	return iKEY->first;
}

//***************************************************************
// *  ��������: �ж��ļ��Ƿ����
// *  ��ڲ���:
// *  ���ڲ���:
// *  �� �� ֵ:
// *  ��������:
// *  �ա�����: 
//***************************************************************
bool CSPIniReadWrite::isFileExist(const String& strPath)
{
    return File(strPath).existsAsFile();
}