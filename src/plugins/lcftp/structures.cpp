#include "structures.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace LCFTP
		{
			bool operator== (const TaskData& t1, const TaskData& t2)
			{
				return t1.ID_ == t2.ID_ &&
					t1.URL_ == t2.URL_ &&
					t1.Filename_ == t2.Filename_;
			}
		};
	};
};

