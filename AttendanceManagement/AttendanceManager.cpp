#include "AttendanceManager.h"

using namespace am;

#pragma region [class] Person
/**
*	�o���Z�X�e�[�^�X�𔽓]������
**/
void Person::SwitchStatus()
{
	if (status) status = false;
	else status = true;
}
/**
*	�ϐ����o�͂���
**/
void Person::print()
{
	std::cout << "index = " << index << std::endl;
	std::cout << "name = " << name << std::endl;
	std::cout << "IDm = " << IDm << std::endl;
	std::cout << "admin = " << admin << std::endl;
	std::cout << "notification = " << notification << std::endl;
	std::cout << "mail_address = " << mail_address << std::endl;
	std::cout << "status = " << status << std::endl;
}
#pragma endregion

#pragma region [class] AttendanceManager

/**<	�f�t�H���g�R���X�g���N�^	*/
AttendanceManager::AttendanceManager()
{
	/*	����������	*/
	if (Initialize() == AM_FAILURE || LoadPersonalDataFile("PersonalData.csv") == AM_FAILURE)
	{
		std::cout << __FUNCTION__ << " error : Initialize() == AM_FAILURE || LoadPersonalDataFile(""PersonalData.csv"") == AM_FAILURE" << std::endl;
		exit(1);
	}
};		
/**<	�f�X�g���N�^	*/
AttendanceManager::~AttendanceManager()
{
	if(m_pasori) pasori_close(m_pasori);
	if(m_felica) felica_free(m_felica);
};	

/**
*	�����o������������
**/
int AttendanceManager::Initialize()
{
	/*	PaSoRi�n���h��������������	*/
	m_pasori = pasori_open(NULL);
	if (!m_pasori)
	{
		std::cerr << __FUNCTION__ << " error : PaSoRi open failed" << std::endl;
		return AM_FAILURE;
	}
	pasori_init(m_pasori);

	/*	PaSoRi�̏�Ԃ��Z�b�g����	*/
	m_pasori_on_card = false;

	return AM_SUCCESS;
}

/**
*	�l�f�[�^�t�@�C����ǂݍ���
*	@param	[in]	std::string filename	�ǂݍ��ރt�@�C����
*	@return ����/���s
*	@note	CSV�t�@�C���O��ł�
**/
int AttendanceManager::LoadPersonalDataFile(std::string filename)
{
	/*	�ǂݍ��ݐ��������	*/
	m_PersonData.clear();

	/*	�t�@�C���I�[�v��	*/
	std::ifstream ifs(filename, std::ios::in);
	if (!ifs.is_open())
	{
		std::cout << __FUNCTION__ << " error : file open error " << filename << std::endl;
		return AM_FAILURE;
	}

	/*	�ǂݍ��ݏ���	*/
	auto PerseLine = [=](std::string line, Person &dst) -> AM_RESULT
	{
		if (line.empty() || line[0] == '#') //	�󕶎�, #�R�����g�s�̓X�L�b�v
			return AM_FAILURE;

		/*	�J���}��؂�ŕ���	*/
		std::vector<std::string> buf;
		SplitStr(line, ",", buf);
		if ((int)buf.size() < 5) return AM_FAILURE;	//	�����͏_���
		
		/*	�f�[�^�ǂݍ���	*/
		dst.index = atoi(buf[0].c_str());	//	�C���f�b�N�X
		dst.name = buf[1];					//	���O
		dst.IDm = buf[2];					//	IDm
		dst.admin = (atoi(buf[3].c_str()) == 1) ? true : false;			//	�Ǘ��Ҍ���
		dst.notification = (atoi(buf[4].c_str()) == 1) ? true : false;	//	�ʒm�t���O
		dst.mail_address = buf[5];			//	EMail�A�h���X
		dst.status = false;					//	�X�e�[�^�X

		/*	�I��	*/
		return AM_SUCCESS;
	};
	std::string line;
	while (std::getline(ifs, line))
	{
		Person p;
		if (PerseLine(line, p) == AM_SUCCESS)
		{
			m_PersonData.push_back(p);
		}
	}

	return AM_SUCCESS;
}

/**
*	�v�b�V�����ꂽ�f�[�^�������ʒm����
*	@note	IC�̓ǂݎ��Ƃ͕ʃX���b�h�Ŏ��s
**/
void AttendanceManager::Notification()
{
	while (1)
	{
		size_t ssize = m_NotificationStack.size();	//	�ʒm�X�^�b�N�̗v�f�����擾
		
		if(ssize)	//	�X�^�b�N���󂶂�Ȃ�������
		{
			Person *p = &m_NotificationStack[ssize - 1];
			if(p->notification) SendMessageEmail(p);		//	���[�����M����
			m_NotificationStack.pop_back();					//	�v�f����폜����
		}
	}
}

/**
*	���b�Z�[�W��ʒm����(���[��)
*	@param	[in]	Person pData	�Ώیl�f�[�^
**/
int AttendanceManager::SendMessageEmail(Person *pData)
{
	/*	���[�N�ϐ�	*/
	std::string bodyFile = "body.txt";

	/*	�ō������擾	*/
	TimeStamp ts = pData->timestamp[pData->timestamp.size() - 1];	//	�����̃f�[�^
	char time_buf[26];
	ctime_s(time_buf, sizeof(time_buf), &ts.time);

	/*	����E�����E�����Z�b�g	*/
	std::string from = "okurinushi@hoge.com";				//	�����	
	std::string to = pData->mail_address;					//	����
	std::string stat = (ts.status) ? "�o�Z" : "���Z";		//	�o���Z
	std::string subject = "�y" + stat + "���m�点���[���z";	//	����

	/*	�{���쐬���t�@�C���o��	*/
	std::ofstream ofs(bodyFile, std::ios::trunc);
	if (!ofs.is_open())
	{
		std::cout << __FUNCTION__ << " error : file open error " << bodyFile << std::endl;
		return AM_FAILURE;
	}
	else
	{
		//	�I���������[���{���I
		ofs << pData->name << " ���� ��" << stat << "���܂����I\n" 
			<< time_buf << std::endl;
		ofs.close();
	}

	/*	�R�}���h����	*/
	std::string cmd = "smail -hsmtp.gmail.com -f" + from + " -s" + subject + " -F" + bodyFile + " " + to;

	/*	���M���s	*/
	std::cout << "sending an e-mail ... ";
	system(cmd.c_str());
	std::cout << "done." << std::endl;

	/*	�{���t�@�C�����폜���ďI��	*/
	std::remove(bodyFile.c_str());

	return AM_SUCCESS;
}

/**
*	IDm���ƍ�����
*	@param	[in]	char[8]	IDm	����IDm
*	@return	�}�b�`����index(�}�b�`���Ȃ����-1)
**/
Person* AttendanceManager::Collation(uint8 IDm[8])
{
	std::string IDmStr = GetIDmStr(IDm);	//	IDm��������擾
	
	/*	���X�g����IDm����v����f�[�^��T��	*/
	Person* ret = nullptr;
	for (auto it = m_PersonData.begin(); it != m_PersonData.end(); it++)
	{
		if (it->IDm == IDmStr)
		{
			ret = (Person *)&(*it);
			break;
		}
	}

	return ret;
}

/**
*	�^�C���J�[�h������
*	@param	[in]	Person	pData	�����l�̃f�[�^
*	@return	����/���s
*	@note	�o�Z/���Z�͊֐����Ŕ���
**/
int AttendanceManager::TimeCardProc(Person *pData)
{
	/*	�L�^�l	*/
	TimeStamp ts;
	ts.time = std::time(nullptr);	//	���������Z�b�g

	/*	�ō��\����	*/
	const double invalid_second = (double)CONSECUTIVE_OPERATION_IGNORE;	//	�����l��IC�A���^�b�`��������(�b)
	size_t ts_size = pData->timestamp.size();
	if (ts_size)
	{
		time_t last = pData->timestamp[ts_size - 1].time;
		if (std::difftime(ts.time, last) < invalid_second)
			return AM_FAILURE;
	}

	/*	�o���Z�̔���	*/
	if (pData->status) ts.status = false;	//	����Ԃ��o�Z�ςȂ牺�Z�����Ƃ��đō�
	else ts.status = true;					//	����Ԃ����Z�ςȂ�o�Z�����Ƃ��đō�

	/*	�f�[�^���v�b�V��	*/
	pData->timestamp.push_back(ts);

	/*	�R���\�[���ɕ\��	*/
	std::string stat = (ts.status) ? "[ �o �Z ]" : "[ �� �Z ]";
	char time_str[26];
	ctime_s(time_str, sizeof(time_str), &ts.time);
	std::cout << stat << " : " << pData->name << " ���� " << time_str;
	
	return AM_SUCCESS;
}

/**
*	IDm�𕶎���ɕϊ�����
*	@return IDm������
**/
std::string AttendanceManager::GetIDmStr(uint8 IDm[8])
{
	std::string ret = "";

	for (int i = 0; i < 8; i++)
	{
		std::ostringstream sout;
		sout << std::hex << std::setfill('0') << std::setw(2) << (int)IDm[i];
		ret += sout.str();
	}

	return ret;
}

/**
*	������𕪊�����
*	@param	[in]	std::string					s		������
*	@param	[in]	std::string					delim	��������
*	@param	[in]	std::vector<std::string>	dst		����
**/
void AttendanceManager::SplitStr(const std::string& s, const std::string& delim, std::vector<std::string> &dst)
{
	dst.clear();

	using string = std::string;
	string::size_type pos = 0;

	while (pos != string::npos)
	{
		string::size_type p = s.find(delim, pos);

		if (p == string::npos)
		{
			dst.push_back(s.substr(pos));
			break;
		}
		else {
			dst.push_back(s.substr(pos, p - pos));
		}

		pos = p + delim.size();
	}
}

/**
*	�ΑӊǗ����J�n����
**/
void AttendanceManager::Run()
{
	/*	�ʒm�X���b�h�X�^�[�g	*/
	std::thread th(&AttendanceManager::Notification, this);

	/*	IC�J�[�h�^�b�`����҂����[�v	*/
	while (1)
	{
		m_felica = felica_polling(m_pasori, POLLING_ANY, 0, 0);	//	�ǂݍ��ݏ���

		if (m_felica)	//	�ǂݍ��߂���
		{
			if (!m_pasori_on_card)	//	'�J�[�h�������'��������
			{
				/*	�f�[�^���ƍ�����	*/
				Person *ptr = Collation(m_felica->IDm);
				if (ptr)	//	�o�^�ς݂̃f�[�^��������
				{
					/*	�ō�����	*/
					if (TimeCardProc(ptr) == AM_SUCCESS)
					{
						/*	�ʒm�X�^�b�N�ɓo�^	*/
						m_NotificationStack.push_back(*ptr);

						/*	�o���Z�X�e�[�^�X��ύX	*/
						ptr->SwitchStatus();
					}
				}
				else //	�f�[�^��������Ȃ�������
				{
					std::cout << "����IC�J�[�h�͓o�^����Ă��܂���B" << std::endl;
				}
				m_pasori_on_card = true;	//	����Ԃ�'�J�[�hON���'��
			}
			else {}	//	'�J�[�hON���'�Ȃ牽�����Ȃ�
		}
		else //	�ǂݍ��߂Ȃ�������
		{
			if (m_pasori_on_card) m_pasori_on_card = false;	//	����Ԃ�'�J�[�h�������'��
		}
	}
}
#pragma endregion