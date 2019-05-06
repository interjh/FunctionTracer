//----------------------------------------------------------------------------------------------------------------------
//  File HolidayEngine/System/Profile/Impl/Windows/Profiler.cpp
//  Created by Junhwan Kim
//  Copyright © 2017년 HolidayGames. All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "Profiler.h"

#include <memory>
#include <atomic>
#include <chrono>

#include <vector>
#include <sstream>
#include <fstream>
#include <unordered_map>
#include <iomanip>
#include <mutex>
#include <thread>
#include <time.h>

namespace HE
{
	namespace Profiler
	{
		struct raw_event
		{
			const char* _name = nullptr;
			const char* _cat = nullptr;
			void* _id = nullptr;
			std::chrono::time_point< std::chrono::system_clock> _ts;
			std::chrono::time_point< std::chrono::system_clock> _endTime;
			uint32_t _pid = 1;
			uint64_t _tid = 0;
		};

		bool s_initialized = false;
		std::string s_prjName;
        std::atomic_bool s_start = {false};
		std::mutex s_mutex;
		std::vector<raw_event> s_storage;
		std::chrono::time_point< std::chrono::system_clock> s_start_ts;

		void Initialize(const char* projectName )
		{
			s_initialized = true;
			s_prjName = projectName;
		}

		void Finalize()
		{
			s_initialized = false;
		}

		/*

{
"traceEvents": [
{ "pid":1, "tid":1, "ts":87705, "dur":956189, "ph":"X", "name":"Jambase", "args":{ "ms":956.2 } },
{ "pid":1, "tid":1, "ts":128154, "dur":75867, "ph":"X", "name":"SyncTargets", "args":{ "ms":75.9 } },
{ "pid":1, "tid":1, "ts":546867, "dur":121564, "ph":"X", "name":"DoThings", "args":{ "ms":121.6 } }
],
"meta_user": "aras",
"meta_cpu_count": "8"
}
*/

		void Flush()
		{
			std::stringstream ss;
			ss << s_prjName;
			time_t t = time(0);
			struct tm* now = localtime(&t);
			ss << '_';
			ss << (now->tm_year + 1900) << '_'
				<< (now->tm_mon + 1) << '_'
				<< now->tm_mday << '_'
				<< now->tm_hour << '_'
				<< now->tm_min << '_'
				<< now->tm_sec;
			ss << ".trace";

			std::lock_guard<std::mutex> lock(s_mutex);
			std::ofstream ofs;
			ofs.open(ss.str(), std::ofstream::out);

			ofs << "{\n\"traceEvents\": [\n";
			for (auto i=0u; i< s_storage.size(); ++i)
			{
				const auto& evt = s_storage[i];
				auto ts = std::chrono::duration_cast<std::chrono::microseconds>(evt._ts - s_start_ts).count();
				auto duration = std::chrono::duration_cast<std::chrono::microseconds>(evt._endTime - evt._ts).count();

				ofs << "{ \"pid\":" << evt._pid << ", " << "\"tid\":" << evt._tid << ", " << "\"ts\":" << ts << ", "
					<< "\"dur\":" << duration << ", " << "\"ph\":\"X\", " << "\"name\":" << "\"" << evt._name << "\"" << ", "
					<< "\"args\": {\"ms\":" << (double)duration / 1000.0f << "} }";

				if (i != (s_storage.size() - 1))
				{
					ofs << ",";
				}

				ofs << std::endl;
			}
			ofs << "]\n}\n" << std::endl;
		}

		std::unordered_map<std::thread::id, std::vector<raw_event>> s_current_event;
		void Start()
		{
			s_start = true;
			s_start_ts = std::chrono::system_clock::now();
			std::lock_guard<std::mutex> lock(s_mutex);
			s_storage = std::vector<raw_event>();
			s_storage.reserve(100000);
		}

		void Stop()
		{
			s_start = false;
			Flush();
		}

		bool IsProfiling()
		{
			return s_start.load();
		}

		void Begin(const char* tag)
		{
			if (s_start)
			{
				auto tid = std::this_thread::get_id();
				std::lock_guard<std::mutex> lock(s_mutex);
				
				raw_event ev;
				ev._name = tag;
				ev._ts = std::chrono::system_clock::now();
				ev._tid = std::hash<std::thread::id>{}(std::this_thread::get_id());
				ev._pid = 1;

				s_current_event[tid].push_back(ev);
			}
		}

		void End()
		{
			if (s_start)
			{
				auto tid = std::this_thread::get_id();
				std::lock_guard<std::mutex> lock(s_mutex);

				auto& stack = s_current_event[tid];
				if (!stack.empty())
				{
					auto& ev = stack.back();
					ev._endTime = std::chrono::system_clock::now();
					if (ev._name)
					{
						s_storage.push_back(ev);
					}
					stack.pop_back();
				}
			}
		}
	}
}
