#include "Time.hpp"
#include "ScriptManager.hpp"

#include <iomanip>
#include <iostream>
#include <sstream>

#include <ctime>

namespace
{
	const auto start = Clock::now();
	const auto sys_start = std::chrono::system_clock::now();
}

Timespan Time::getClockPrecision()
{
	Timespan lowest = std::chrono::seconds(10);

	for (int i = 0; i < 1000; ++i)
	{
		Timestamp start = Clock::now();
		Timestamp now;
		do
		{
			now = Clock::now();
		} while (now == start);

		if (lowest > now - start)
			lowest = now - start;
	}

	return lowest;
}
Timespan Time::getRunTime()
{
	return Clock::now() - start;
}

std::ostream& operator<<(std::ostream& os, const Timestamp& time)
{
	using std::chrono::duration_cast;
	using std::chrono::milliseconds;
	using std::chrono::system_clock;
	
	auto epoch = duration_cast<system_clock::duration>(time - start);
	std::time_t timeval = system_clock::to_time_t(sys_start + epoch);
	std::tm tm;

#if defined __STDC_LIB_EXT1__ || defined _MSC_VER
	localtime_s(&tm, &timeval);
#else
	tm = *localtime(&timeval);
#endif

	auto prev = os.fill('0');
	os << std::setw(4) << (1900 + tm.tm_year) << "-"
		<< std::setw(2) << (tm.tm_mon + 1) << "-"
		<< std::setw(2) << tm.tm_mday << " "
		<< std::setw(2) << tm.tm_hour << ":"
		<< std::setw(2) << tm.tm_min << ":"
		<< std::setw(2) << tm.tm_sec;
	os.fill(prev);

	return os;
}
std::ostream& operator<<(std::ostream& os, const Timespan& dur)
{
	using std::chrono::duration_cast;
	using std::chrono::nanoseconds;
	typedef std::chrono::duration<float, std::micro> microseconds;
	typedef std::chrono::duration<float, std::milli> milliseconds;
	typedef std::chrono::duration<float> seconds;

	auto prev = os.precision(2);
	auto prevf = os.setf(std::ios_base::fixed);
	if (dur <= std::chrono::microseconds(1))
		os << duration_cast<nanoseconds>(dur).count() << "ns";
	else if (dur <= std::chrono::milliseconds(1))
		os << duration_cast<microseconds>(dur).count() << "us";
	else if (dur <= std::chrono::seconds(1))
		os << duration_cast<milliseconds>(dur).count() << "ms";
	else if (dur <= std::chrono::seconds(60))
		os << duration_cast<seconds>(dur).count() << "s";
	else if (dur <= std::chrono::seconds(3600))
		os << duration_cast<seconds>(dur).count() / 60 << "m";
	else
		os << duration_cast<seconds>(dur).count() / 3600 << "h";
		
	os.precision(prev);
	os.setf(prevf);

	return os;
}

namespace
{
	void createTimestamp(void* mem)
	{
		new(mem) Timestamp;
	}
	void destroyTimestamp(Timestamp* mem)
	{
		mem->~time_point();
	}
	void createTimespan(void* mem)
	{
		new(mem) Timespan;
	}
	void destroyTimespan(Timespan* mem)
	{
		mem->~duration();
	}

	void opSub(asIScriptGeneric* gen)
	{
		const Timestamp* a = (const Timestamp*)gen->GetObject();
		const Timestamp* b = (const Timestamp*)gen->GetArgObject(0);

		new (gen->GetAddressOfReturnLocation()) Timespan(*a - *b);
	}
	void opAdd(asIScriptGeneric* gen)
	{
		const Timestamp* a = (const Timestamp*)gen->GetObject();
		const Timespan* b = (const Timespan*)gen->GetArgObject(0);

		new (gen->GetAddressOfReturnLocation()) Timestamp(*a + *b);
	}

	std::string toStringStamp(const Timestamp& s)
	{
		std::ostringstream oss;
		oss << s;
		return oss.str();
	}
	std::string toStringSpan(const Timespan& s)
	{
		std::ostringstream oss;
		oss << s;
		return oss.str();
	}

	int64_t nanoseconds(const Timespan& s) { return std::chrono::duration_cast<std::chrono::nanoseconds>(s).count(); }
	int64_t milliseconds(const Timespan& s) { return std::chrono::duration_cast<std::chrono::milliseconds>(s).count(); }
	float seconds(const Timespan& s) { return std::chrono::duration_cast<std::chrono::duration<float>>(s).count(); }
	void fromNanoseconds(asIScriptGeneric* gen)
	{
		int64_t ns = gen->GetArgQWord(0);
		new (gen->GetAddressOfReturnLocation()) Timespan(std::chrono::nanoseconds(ns));
	}
	void fromMilliseconds(asIScriptGeneric* gen)
	{
		int64_t ms = gen->GetArgQWord(0);
		new (gen->GetAddressOfReturnLocation()) Timespan(std::chrono::milliseconds(ms));
	}
	void fromSeconds(asIScriptGeneric* gen)
	{
		float s = gen->GetArgFloat(0);
		new (gen->GetAddressOfReturnLocation()) Timespan(std::chrono::duration_cast<Timespan>(std::chrono::duration<float>(s)));
	}

	int opCmp(const Timestamp& a, const Timestamp& b)
	{
		if (a < b)
			return -1;
		else if (a > b)
			return 1;
		return 0;
	}
	int opCmpSpan(const Timespan& a, const Timespan& b)
	{
		if (a < b)
			return -1;
		else if (a > b)
			return 1;
		return 0;
	}

	void getTimeAt(asIScriptGeneric* gen)
	{
		std::tm t;
		
		t.tm_hour = gen->GetArgDWord(3);
		t.tm_min = gen->GetArgDWord(4);
		t.tm_sec = gen->GetArgDWord(5);
		t.tm_mday = gen->GetArgDWord(2);
		t.tm_mon = gen->GetArgDWord(1) - 1;
		t.tm_year = gen->GetArgDWord(0) - 1900;

		auto time = std::mktime(&t);

		Timestamp point = start + (std::chrono::system_clock::from_time_t(time) - sys_start);

		new (gen->GetAddressOfReturnLocation()) Timestamp(point);
	}

	void getNow(asIScriptGeneric* gen)
	{
		new (gen->GetAddressOfReturnLocation()) Timestamp(Clock::now());
	}
	void getTotal(asIScriptGeneric* gen)
	{
		new (gen->GetAddressOfReturnLocation()) Timespan(Clock::now() - start);
	}
}

void Time::registerTimeTypes(ScriptManager& man)
{
	man.addExtension("Time", [](asIScriptEngine* eng) {
		AS_ASSERT(eng->RegisterObjectType("Timestamp", sizeof(Timestamp), asOBJ_VALUE | asGetTypeTraits<Timestamp>()));
		AS_ASSERT(eng->RegisterObjectType("Timespan", sizeof(Timespan), asOBJ_VALUE | asGetTypeTraits<Timespan>()));

		AS_ASSERT(eng->RegisterObjectBehaviour("Timestamp", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(createTimestamp), asCALL_CDECL_OBJFIRST));
		AS_ASSERT(eng->RegisterObjectBehaviour("Timestamp", asBEHAVE_DESTRUCT, "void f()", asFUNCTION(destroyTimestamp), asCALL_CDECL_OBJFIRST));

		AS_ASSERT(eng->RegisterObjectMethod("Timestamp", "Timestamp& opAssign(const Timestamp&in)", asMETHODPR(Timestamp, operator=, (const Timestamp&), Timestamp&), asCALL_THISCALL));

		AS_ASSERT(eng->RegisterObjectMethod("Timestamp", "Timestamp opAdd(const Timespan&in) const", asFUNCTION(opAdd), asCALL_GENERIC));
		AS_ASSERT(eng->RegisterObjectMethod("Timestamp", "Timespan opSub(const Timestamp&in) const", asFUNCTION(opSub), asCALL_GENERIC));
		AS_ASSERT(eng->RegisterObjectMethod("Timestamp", "int opCmp(const Timestamp&in) const", asFUNCTION(opCmp), asCALL_CDECL_OBJFIRST));

		AS_ASSERT(eng->RegisterObjectMethod("Timestamp", "string opConv() const", asFUNCTION(toStringStamp), asCALL_CDECL_OBJFIRST));
		AS_ASSERT(eng->RegisterObjectMethod("Timestamp", "string ToString() const", asFUNCTION(toStringStamp), asCALL_CDECL_OBJFIRST));

		AS_ASSERT(eng->RegisterObjectBehaviour("Timespan", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(createTimespan), asCALL_CDECL_OBJFIRST));
		AS_ASSERT(eng->RegisterObjectBehaviour("Timespan", asBEHAVE_DESTRUCT, "void f()", asFUNCTION(destroyTimespan), asCALL_CDECL_OBJFIRST));

		AS_ASSERT(eng->RegisterObjectMethod("Timespan", "Timespan& opAssign(const Timespan&in)", asMETHODPR(Timespan, operator=, (const Timespan&), Timespan&), asCALL_THISCALL));
		AS_ASSERT(eng->RegisterObjectMethod("Timespan", "Timespan& opAddAssign(const Timespan&in) const", asMETHODPR(Timespan, operator+=, (const Timespan&), Timespan&), asCALL_THISCALL));
		AS_ASSERT(eng->RegisterObjectMethod("Timespan", "int opCmp(const Timespan&in) const", asFUNCTION(opCmpSpan), asCALL_CDECL_OBJFIRST));

		AS_ASSERT(eng->RegisterObjectMethod("Timespan", "int64 get_Count() const", asMETHOD(Timespan, count), asCALL_THISCALL));
		AS_ASSERT(eng->RegisterObjectMethod("Timespan", "int64 get_Nanoseconds() const", asFUNCTION(nanoseconds), asCALL_CDECL_OBJFIRST));
		AS_ASSERT(eng->RegisterObjectMethod("Timespan", "int64 get_Milliseconds() const", asFUNCTION(milliseconds), asCALL_CDECL_OBJFIRST));
		AS_ASSERT(eng->RegisterObjectMethod("Timespan", "float get_Seconds() const", asFUNCTION(seconds), asCALL_CDECL_OBJFIRST));

		AS_ASSERT(eng->RegisterObjectMethod("Timespan", "string opConv() const", asFUNCTION(toStringSpan), asCALL_CDECL_OBJFIRST));
		AS_ASSERT(eng->RegisterObjectMethod("Timespan", "string ToString() const", asFUNCTION(toStringSpan), asCALL_CDECL_OBJFIRST));

		AS_ASSERT(eng->SetDefaultNamespace("Time"));
		AS_ASSERT(eng->RegisterGlobalFunction("::Timestamp get_Now()", asFUNCTION(getNow), asCALL_GENERIC));
		AS_ASSERT(eng->RegisterGlobalFunction("::Timespan get_Total()", asFUNCTION(getTotal), asCALL_GENERIC));

		AS_ASSERT(eng->RegisterGlobalFunction("::Timestamp At(uint year, uint mon, uint day, uint h = 0, uint m = 0, uint s = 0)", asFUNCTION(getTimeAt), asCALL_GENERIC));
		AS_ASSERT(eng->RegisterGlobalFunction("::Timespan Nanoseconds(int64)", asFUNCTION(fromNanoseconds), asCALL_GENERIC));
		AS_ASSERT(eng->RegisterGlobalFunction("::Timespan Milliseconds(int64)", asFUNCTION(fromMilliseconds), asCALL_GENERIC));
		AS_ASSERT(eng->RegisterGlobalFunction("::Timespan Seconds(float)", asFUNCTION(fromSeconds), asCALL_GENERIC));
		AS_ASSERT(eng->SetDefaultNamespace(""));
	});

	man.registerSerializedType<Timestamp>("Timestamp");
	man.registerSerializedType<Timespan>("Timespan");
}

// Let's try to keep Windows.h as far out of the way as possible
#ifdef NEEDS_HIGH_RESOLUTION_CLOCK
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace
{
	static LARGE_INTEGER _Freq;
}

Clock::time_point Clock::now()
{
	if (_Freq.QuadPart == 0)
		QueryPerformanceFrequency(&_Freq);

	LARGE_INTEGER _Ctr;
	QueryPerformanceCounter(&_Ctr);
	static_assert(period::num == 1, "This assumes period::num == 1.");
	const long long _Whole = (_Ctr.QuadPart / _Freq.QuadPart) * period::den;
	const long long _Part = (_Ctr.QuadPart % _Freq.QuadPart) * period::den / _Freq.QuadPart;
	return (time_point(duration(_Whole + _Part)));
}
#endif
