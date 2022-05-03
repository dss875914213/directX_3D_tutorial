#pragma  once
#include <windows.h>

class Object
{

};

template<typename returnType, typename argsType>
class Event
{
#define EVENT_LIST_MAX_NUM (10)
	typedef returnType(Object::* pMemFunc)(argsType arg);
public:
	Event()
	{
		m_totalFunc = 0;
		m_obj = NULL;
		for (int i = 0; i < EVENT_LIST_MAX_NUM; i++)
			m_func[i] = NULL;
	}

	template<class funcType>
	void associate(Object* obj, funcType func)
	{
		m_obj = obj;
		m_func[m_totalFunc] = static_cast<pMemFunc>(func);
		m_totalFunc++;
	}

	template<class funcType>
	void disAssociate(Object* obj, funcType func)
	{
		bool isFind = false;
		int i = 0;
		if (obj != m_obj)
			return;

		for (i = 0; i < m_totalFunc; i++)
		{
			if (m_func[i] == func)
			{
				isFind = true;
				break;
			}
		}

		if (isFind)
		{
			for (i; i < m_totalFunc - 1; i++)
				m_func[i] = m_func[i + 1];

			m_func[i] = NULL;
			m_totalFunc--;
		}
	}

	void sendEvent(argsType arg)
	{
		for (int i = 0; i < m_totalFunc; i++)
		{
			if (m_func[i] != NULL)
				((m_obj->*pMemFunc(m_func[i])))(arg);
		}
	}

private:
	Object* m_obj;
	pMemFunc m_func[EVENT_LIST_MAX_NUM];
	int m_totalFunc;
};
