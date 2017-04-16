#pragma once

#include "common.h"

#define PERSONAL_DATA_CSV "personal_data.csv"
#define CONSECUTIVE_OPERATION_IGNORE 10

namespace am
{
	/**
	*	@enum	AM_RESULT
	*	@brief	処理結果
	**/
	enum AM_RESULT
	{
		AM_SUCCESS, AM_FAILURE
	};

	/**
	*	@struct TimeStamp
	*	@brief	タイムスタンプ構造体
	**/
	typedef struct TimeStamp
	{
		time_t	time;	/**<	時刻	*/
		bool	status;	/**<	true:登校時刻, false:下校時刻	*/

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
	*	@brief	個人データ
	**/
	class Person
	{
	public:
		int			index;			/**<	インデックス	*/
		std::string name;			/**<	名前	*/
		std::string	IDm;			/**<	ICカードID	*/
		bool		admin;			/**<	管理者フラグ	*/
		bool		notification;	/**<	通知する/しない	*/
		std::string mail_address;	/**<	メールアドレス	*/

		bool					status;			/**<	登校済/下校済	*/
		std::vector<TimeStamp>	timestamp;		/**<	タイムスタンプの配列	*/
	
	public:
		/**
		*	登下校ステータスを反転させる
		**/
		void SwitchStatus();

		/**
		*	変数を出力する
		**/
		void print();
	};

	/**
	*	@class	AttendanceManager
	*	@brief	勤怠管理クラス
	**/
	class AttendanceManager
	{
	private:

		pasori *m_pasori;	/**<	PaSoRiハンドラ	*/
		felica *m_felica;	/**<	felicaハンドラ	*/
		bool m_pasori_on_card;		/**<	PaSoRiカードフラグ	*/

		std::vector<Person>	m_PersonData;			/**<	個人データの配列	*/
		std::vector<Person> m_NotificationStack;	/**<	通知用のスタック	*/

		/**
		*	メンバを初期化する
		*	@return	成功/失敗
		**/
		int Initialize();

		/**
		*	個人データファイルを読み込む
		*	@param	[in]	std::string filename	読み込むファイル名
		*	@return 成功/失敗
		*	@note	CSVファイル前提です
		**/
		int LoadPersonalDataFile(std::string filename);

		/**
		*	プッシュされたデータを順次通知する
		*	@note	ICの読み取りとは別スレッドで実行
		**/
		void Notification();

		/**
		*	メッセージを通知する(メール)
		*	@param	[in]	Person pData	送り先の個人データ
		**/
		int SendMessageEmail(Person *pData);

		/**
		*	IDmを照合する
		*	@param	[in]	char[8]	IDm	入力IDm
		*	@return	マッチしたデータへのポインタ(マッチしなければnullptr)
		**/
		Person* Collation(uint8 IDm[8]);

		/**
		*	タイムカードを押す
		*	@param	[in]	Person	pData	押す人のデータ
		*	@return	成功/失敗
		*	@note	登校/下校は関数内で判定
		**/
		int TimeCardProc(Person *pData);

		/**
		*	IDmを文字列に変換する
		*	@return IDm文字列
		**/
		std::string GetIDmStr(uint8 IDm[8]);

		/**
		*	文字列を分割する
		*	@param	[in]	std::string					s		文字列
		*	@param	[in]	std::string					delim	分割文字
		*	@param	[in]	std::vector<std::string>	dst		結果
		**/
		void SplitStr(const std::string& s, const std::string& delim, std::vector<std::string> &dst);
		 
	public:
		AttendanceManager();	/**<	デフォルトコンストラクタ	*/
		~AttendanceManager();	/**<	デストラクタ	*/

		/**
		*	勤怠管理を開始する
		**/
		void Run();
	};
}

