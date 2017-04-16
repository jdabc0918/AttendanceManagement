#pragma once

#include "common.h"

#define PERSONAL_DATA_CSV "personal_data.csv"
#define CONSECUTIVE_OPERATION_IGNORE 10

namespace am
{
	/**
	*	@enum	AM_RESULT
	*	@brief	��������
	**/
	enum AM_RESULT
	{
		AM_SUCCESS, AM_FAILURE
	};

	/**
	*	@struct TimeStamp
	*	@brief	�^�C���X�^���v�\����
	**/
	typedef struct TimeStamp
	{
		time_t	time;	/**<	����	*/
		bool	status;	/**<	true:�o�Z����, false:���Z����	*/

		TimeStamp() {};
		TimeStamp(const TimeStamp& obj)
		{
			time = obj.time;
			status = obj.status;
		}
		~TimeStamp() {};

	}TimeStamp;

	/**
	*	@class	Person
	*	@brief	�l�f�[�^
	**/
	class Person
	{
	public:
		int			index;			/**<	�C���f�b�N�X	*/
		std::string name;			/**<	���O	*/
		std::string	IDm;			/**<	IC�J�[�hID	*/
		bool		admin;			/**<	�Ǘ��҃t���O	*/
		bool		notification;	/**<	�ʒm����/���Ȃ�	*/
		std::string mail_address;	/**<	���[���A�h���X	*/

		bool					status;			/**<	�o�Z��/���Z��	*/
		std::vector<TimeStamp>	timestamp;		/**<	�^�C���X�^���v�̔z��	*/
	
	public:
		/**
		*	�o���Z�X�e�[�^�X�𔽓]������
		**/
		void SwitchStatus();

		/**
		*	�ϐ����o�͂���
		**/
		void print();
	};

	/**
	*	@class	AttendanceManager
	*	@brief	�ΑӊǗ��N���X
	**/
	class AttendanceManager
	{
	private:

		pasori *m_pasori;	/**<	PaSoRi�n���h��	*/
		felica *m_felica;	/**<	felica�n���h��	*/
		bool m_pasori_on_card;		/**<	PaSoRi�J�[�h�t���O	*/

		std::vector<Person>	m_PersonData;			/**<	�l�f�[�^�̔z��	*/
		std::vector<Person> m_NotificationStack;	/**<	�ʒm�p�̃X�^�b�N	*/

		/**
		*	�����o������������
		*	@return	����/���s
		**/
		int Initialize();

		/**
		*	�l�f�[�^�t�@�C����ǂݍ���
		*	@param	[in]	std::string filename	�ǂݍ��ރt�@�C����
		*	@return ����/���s
		*	@note	CSV�t�@�C���O��ł�
		**/
		int LoadPersonalDataFile(std::string filename);

		/**
		*	�v�b�V�����ꂽ�f�[�^�������ʒm����
		*	@note	IC�̓ǂݎ��Ƃ͕ʃX���b�h�Ŏ��s
		**/
		void Notification();

		/**
		*	���b�Z�[�W��ʒm����(���[��)
		*	@param	[in]	Person pData	�����̌l�f�[�^
		**/
		int SendMessageEmail(Person *pData);

		/**
		*	IDm���ƍ�����
		*	@param	[in]	char[8]	IDm	����IDm
		*	@return	�}�b�`�����f�[�^�ւ̃|�C���^(�}�b�`���Ȃ����nullptr)
		**/
		Person* Collation(uint8 IDm[8]);

		/**
		*	�^�C���J�[�h������
		*	@param	[in]	Person	pData	�����l�̃f�[�^
		*	@return	����/���s
		*	@note	�o�Z/���Z�͊֐����Ŕ���
		**/
		int TimeCardProc(Person *pData);

		/**
		*	IDm�𕶎���ɕϊ�����
		*	@return IDm������
		**/
		std::string GetIDmStr(uint8 IDm[8]);

		/**
		*	������𕪊�����
		*	@param	[in]	std::string					s		������
		*	@param	[in]	std::string					delim	��������
		*	@param	[in]	std::vector<std::string>	dst		����
		**/
		void SplitStr(const std::string& s, const std::string& delim, std::vector<std::string> &dst);
		 
	public:
		AttendanceManager();	/**<	�f�t�H���g�R���X�g���N�^	*/
		~AttendanceManager();	/**<	�f�X�g���N�^	*/

		/**
		*	�ΑӊǗ����J�n����
		**/
		void Run();
	};
}

