#include "AttendanceManager.h"

using namespace am;

#pragma region [class] Person
/**
*	登下校ステータスを反転させる
**/
void Person::SwitchStatus()
{
	if (status) status = false;
	else status = true;
}
/**
*	変数を出力する
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

/**<	デフォルトコンストラクタ	*/
AttendanceManager::AttendanceManager()
{
	/*	初期化処理	*/
	if (Initialize() == AM_FAILURE || LoadPersonalDataFile("PersonalData.csv") == AM_FAILURE)
	{
		std::cout << __FUNCTION__ << " error : Initialize() == AM_FAILURE || LoadPersonalDataFile(""PersonalData.csv"") == AM_FAILURE" << std::endl;
		exit(1);
	}
};		
/**<	デストラクタ	*/
AttendanceManager::~AttendanceManager()
{
	if(m_pasori) pasori_close(m_pasori);
	if(m_felica) felica_free(m_felica);
};	

/**
*	メンバを初期化する
**/
int AttendanceManager::Initialize()
{
	/*	PaSoRiハンドラを初期化する	*/
	m_pasori = pasori_open(NULL);
	if (!m_pasori)
	{
		std::cerr << __FUNCTION__ << " error : PaSoRi open failed" << std::endl;
		return AM_FAILURE;
	}
	pasori_init(m_pasori);

	/*	PaSoRiの状態をセットする	*/
	m_pasori_on_card = false;

	return AM_SUCCESS;
}

/**
*	個人データファイルを読み込む
*	@param	[in]	std::string filename	読み込むファイル名
*	@return 成功/失敗
*	@note	CSVファイル前提です
**/
int AttendanceManager::LoadPersonalDataFile(std::string filename)
{
	/*	読み込み先を初期化	*/
	m_PersonData.clear();

	/*	ファイルオープン	*/
	std::ifstream ifs(filename, std::ios::in);
	if (!ifs.is_open())
	{
		std::cout << __FUNCTION__ << " error : file open error " << filename << std::endl;
		return AM_FAILURE;
	}

	/*	読み込み処理	*/
	auto PerseLine = [=](std::string line, Person &dst) -> AM_RESULT
	{
		if (line.empty() || line[0] == '#') //	空文字, #コメント行はスキップ
			return AM_FAILURE;

		/*	カンマ区切りで分割	*/
		std::vector<std::string> buf;
		SplitStr(line, ",", buf);
		if ((int)buf.size() < 5) return AM_FAILURE;	//	ここは柔軟に
		
		/*	データ読み込み	*/
		dst.index = atoi(buf[0].c_str());	//	インデックス
		dst.name = buf[1];					//	名前
		dst.IDm = buf[2];					//	IDm
		dst.admin = (atoi(buf[3].c_str()) == 1) ? true : false;			//	管理者権限
		dst.notification = (atoi(buf[4].c_str()) == 1) ? true : false;	//	通知フラグ
		dst.mail_address = buf[5];			//	EMailアドレス
		dst.status = false;					//	ステータス

		/*	終了	*/
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
*	プッシュされたデータを順次通知する
*	@note	ICの読み取りとは別スレッドで実行
**/
void AttendanceManager::Notification()
{
	while (1)
	{
		size_t ssize = m_NotificationStack.size();	//	通知スタックの要素数を取得
		
		if(ssize)	//	スタックが空じゃなかったら
		{
			Person *p = &m_NotificationStack[ssize - 1];
			if(p->notification) SendMessageEmail(p);		//	メール送信処理
			m_NotificationStack.pop_back();					//	要素を一つ削除する
		}
	}
}

/**
*	メッセージを通知する(メール)
*	@param	[in]	Person pData	対象個人データ
**/
int AttendanceManager::SendMessageEmail(Person *pData)
{
	/*	ワーク変数	*/
	std::string bodyFile = "body.txt";

	/*	打刻情報を取得	*/
	TimeStamp ts = pData->timestamp[pData->timestamp.size() - 1];	//	末尾のデータ
	char time_buf[26];
	ctime_s(time_buf, sizeof(time_buf), &ts.time);

	/*	宛先・件名・送り主セット	*/
	std::string from = "okurinushi@hoge.com";				//	送り主	
	std::string to = pData->mail_address;					//	宛先
	std::string stat = (ts.status) ? "登校" : "下校";		//	登下校
	std::string subject = "【" + stat + "お知らせメール】";	//	件名

	/*	本文作成＆ファイル出力	*/
	std::ofstream ofs(bodyFile, std::ios::trunc);
	if (!ofs.is_open())
	{
		std::cout << __FUNCTION__ << " error : file open error " << bodyFile << std::endl;
		return AM_FAILURE;
	}
	else
	{
		//	！ここがメール本文！
		ofs << pData->name << " さん が" << stat << "しました！\n" 
			<< time_buf << std::endl;
		ofs.close();
	}

	/*	コマンド生成	*/
	std::string cmd = "smail -hsmtp.gmail.com -f" + from + " -s" + subject + " -F" + bodyFile + " " + to;

	/*	送信実行	*/
	std::cout << "sending an e-mail ... ";
	system(cmd.c_str());
	std::cout << "done." << std::endl;

	/*	本文ファイルを削除して終了	*/
	std::remove(bodyFile.c_str());

	return AM_SUCCESS;
}

/**
*	IDmを照合する
*	@param	[in]	char[8]	IDm	入力IDm
*	@return	マッチしたindex(マッチしなければ-1)
**/
Person* AttendanceManager::Collation(uint8 IDm[8])
{
	std::string IDmStr = GetIDmStr(IDm);	//	IDm文字列を取得
	
	/*	リスト中のIDmが一致するデータを探す	*/
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
*	タイムカードを押す
*	@param	[in]	Person	pData	押す人のデータ
*	@return	成功/失敗
*	@note	登校/下校は関数内で判定
**/
int AttendanceManager::TimeCardProc(Person *pData)
{
	/*	記録値	*/
	TimeStamp ts;
	ts.time = std::time(nullptr);	//	現時刻をセット

	/*	打刻可能判定	*/
	const double invalid_second = (double)CONSECUTIVE_OPERATION_IGNORE;	//	同じ人のIC連続タッチ無視時間(秒)
	size_t ts_size = pData->timestamp.size();
	if (ts_size)
	{
		time_t last = pData->timestamp[ts_size - 1].time;
		if (std::difftime(ts.time, last) < invalid_second)
			return AM_FAILURE;
	}

	/*	登下校の判定	*/
	if (pData->status) ts.status = false;	//	現状態が登校済なら下校時刻として打刻
	else ts.status = true;					//	現状態が下校済なら登校時刻として打刻

	/*	データをプッシュ	*/
	pData->timestamp.push_back(ts);

	/*	コンソールに表示	*/
	std::string stat = (ts.status) ? "[ 登 校 ]" : "[ 下 校 ]";
	char time_str[26];
	ctime_s(time_str, sizeof(time_str), &ts.time);
	std::cout << stat << " : " << pData->name << " さん " << time_str;
	
	return AM_SUCCESS;
}

/**
*	IDmを文字列に変換する
*	@return IDm文字列
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
*	文字列を分割する
*	@param	[in]	std::string					s		文字列
*	@param	[in]	std::string					delim	分割文字
*	@param	[in]	std::vector<std::string>	dst		結果
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
*	勤怠管理を開始する
**/
void AttendanceManager::Run()
{
	/*	通知スレッドスタート	*/
	std::thread th(&AttendanceManager::Notification, this);

	/*	ICカードタッチ操作待ちループ	*/
	while (1)
	{
		m_felica = felica_polling(m_pasori, POLLING_ANY, 0, 0);	//	読み込み処理

		if (m_felica)	//	読み込めたら
		{
			if (!m_pasori_on_card)	//	'カード無し状態'だったら
			{
				/*	データを照合する	*/
				Person *ptr = Collation(m_felica->IDm);
				if (ptr)	//	登録済みのデータだったら
				{
					/*	打刻処理	*/
					if (TimeCardProc(ptr) == AM_SUCCESS)
					{
						/*	通知スタックに登録	*/
						m_NotificationStack.push_back(*ptr);

						/*	登下校ステータスを変更	*/
						ptr->SwitchStatus();
					}
				}
				else //	データが見つからなかったら
				{
					std::cout << "このICカードは登録されていません。" << std::endl;
				}
				m_pasori_on_card = true;	//	現状態を'カードON状態'に
			}
			else {}	//	'カードON状態'なら何もしない
		}
		else //	読み込めなかったら
		{
			if (m_pasori_on_card) m_pasori_on_card = false;	//	現状態を'カード無し状態'に
		}
	}
}
#pragma endregion