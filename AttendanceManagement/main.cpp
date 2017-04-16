
#include "common.h"
#include "AttendanceManager.h"

int _tmain(int argc, _TCHAR *argv[])
{
	using namespace am;

	std::cout << "★★★ IC登下校メールシステム開始 ★★★" << std::endl;

	AttendanceManager *manager = new AttendanceManager();
	manager->Run();

	return 0;
}
