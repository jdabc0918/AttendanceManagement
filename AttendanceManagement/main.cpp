
#include "common.h"
#include "AttendanceManager.h"

int _tmain(int argc, _TCHAR *argv[])
{
	using namespace am;

	std::cout << "������ IC�o���Z���[���V�X�e���J�n ������" << std::endl;

	AttendanceManager *manager = new AttendanceManager();
	manager->Run();

	return 0;
}
