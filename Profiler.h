//---------------------------------------------------------------------------------------------------------
//  File HolidayEngine/System/Profile/Profiler.h
//  Created by Junhwan Kim
//  Copyright 2018 HolidayGames. All rights reserved.
//---------------------------------------------------------------------------------------------------------

#ifndef _HOLIDAYENGINE_SYSTEM_PROFILE_PROFILER_H_
#define _HOLIDAYENGINE_SYSTEM_PROFILE_PROFILER_H_

#define HE_PROFILE 1

namespace HE
{
	namespace Profiler
	{
		void Initialize(const char* projectName);
		void Finalize();

		void Start();
		void Stop();
		bool IsProfiling(); 

		void Begin(const char* tag);

		void End();

		struct ScopedZone
		{
			ScopedZone(const char* tag) { Begin(tag); }
			~ScopedZone(){ End(); }
            
            ScopedZone(const ScopedZone& other) = delete;
            ScopedZone(ScopedZone&& other) = delete;
            ScopedZone& operator = (const ScopedZone &other) = delete;
            ScopedZone& operator = (ScopedZone&& other) = delete;
		};
	}
}

#if HE_PROFILE
	#define HE_PROFILER_INITIALIZE(projectName) ::HE::Profiler::Initialize(projectName);
	#define HE_PROFILER_FINALIZE ::HE::Profiler::Finalize()
	#define HE_PROFILER_BEGIN(TAG) ::HE::Profiler::Begin(TAG)
	#define HE_PROFILER_END ::HE::Profiler::End()
	#define HE_PROFILER_SCOPED(TAG) ::HE::Profiler::ScopedZone HE_LINE_ID(scopedZone)(TAG)
#else
	#define HE_PROFILER_INITIALIZE
	#define HE_PROFILER_FINALIZE
	#define HE_PROFILER_BEGIN(TAG)
	#define HE_PROFILER_END()
	#define HE_PROFILER_SCOPED(TAG)
#endif

#endif
