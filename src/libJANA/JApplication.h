//
//    File: JApplication.h
// Created: Wed Oct 11 13:09:35 EDT 2017
// Creator: davidl (on Darwin harriet.jlab.org 15.6.0 i386)
//
// ------ Last repository commit info -----
// [ Date ]
// [ Author ]
// [ Source ]
// [ Revision ]
//
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
// Jefferson Science Associates LLC Copyright Notice:  
// Copyright 251 2014 Jefferson Science Associates LLC All Rights Reserved. Redistribution
// and use in source and binary forms, with or without modification, are permitted as a
// licensed user provided that the following conditions are met:  
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer. 
// 2. Redistributions in binary form must reproduce the above copyright notice, this
//    list of conditions and the following disclaimer in the documentation and/or other
//    materials provided with the distribution.  
// 3. The name of the author may not be used to endorse or promote products derived
//    from this software without specific prior written permission.  
// This material resulted from work developed under a United States Government Contract.
// The Government retains a paid-up, nonexclusive, irrevocable worldwide license in such
// copyrighted data to reproduce, distribute copies to the public, prepare derivative works,
// perform publicly and display publicly and to permit others to do so.   
// THIS SOFTWARE IS PROVIDED BY JEFFERSON SCIENCE ASSOCIATES LLC "AS IS" AND ANY EXPRESS
// OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL
// JEFFERSON SCIENCE ASSOCIATES, LLC OR THE U.S. GOVERNMENT BE LIABLE TO LICENSEE OR ANY
// THIRD PARTES FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
// OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

#ifndef _JApplication_h_
#define _JApplication_h_

#include <cstdint>
#include <vector>
using std::vector;

class JEventProcessor;
class JEventSource;
class JEventSourceGenerator;
class JFactoryGenerator;
class JQueue;
class JParameterManager;
class JResourceManager;
class JThread;
class JTask;

class JApplication{
	public:
		JApplication();
		virtual ~JApplication();

		int GetExitCode(void);
		void Quit(void);
		void Run(uint32_t nthreads=0);
		void SetMaxThreads(uint32_t);
		void SetTicker(bool ticker_on=true);
		void Stop(void);
		
		void AddJEventProcessor(JEventProcessor *processor);
		void AddJEventSource(JEventSource *source);
		void AddJEventSourceGenerator(JEventSourceGenerator *source_generator);
		void AddJFactoryGenerator(JFactoryGenerator *factory_generator);
		
		void GetJEventProcessors(vector<JEventProcessor*> &processors);
		void GetJEventSources(vector<JEventSource*> &sources);
		void GetJEventSourceGenerators(vector<EventSourceGenerator*> &source_generators);
		void GetJFactoryGenerators(vector<JFactoryGenerator*> &factory_generators);
		void GetJQueues(vector<const JQueue*> &queues);
		const JQueue* GetJQueue(const string &name);
		const JParameterManager* GetJParameterManager(void);
		const JResourceManager* GetJResourceManager(void);
		
		uint32_t GetNcores(void);
		uint32_t GetNJThreads(void);
		uint64_t GetNtasksCompleted(string name="");
		uint64_t GetNeventsProcessed(void);
		float GetIntegratedRate(void);
		float GetInstantaneousRate(void);
		void GetInstantaneousRates(vector<double> &rates_by_queue);
		void GetIntegratedRates(map<string,double> &rates_by_thread);
		
		void RemoveJEventProcessor(JEvenetProcessor *processor);
		void RemoveJEventSource(JEventSource *source);
		void RemoveJEventSourceGenerator(JEventSourceGenerator *source_generator);
		void RemoveJFactoryGenerator(JFactoryGenerator *factory_generator);

	protected:
		
	private:

};

#endif // _JApplication_h_
