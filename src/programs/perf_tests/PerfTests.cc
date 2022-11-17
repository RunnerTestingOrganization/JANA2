
// Copyright 2022, Jefferson Science Associates, LLC.
// Subject to the terms in the LICENSE file found in the top-level directory.


// PerfTests is designed to make it easy to see how changes to JANA's internals affect its performance on
// a variety of workloads we care about.

#include <JANA/JApplication.h>

#include <iostream>
#include <thread>

int main() {

    auto params = new JParameterManager;
    params->SetParameter("log:off", "JApplication,JPluginLoader"); // Log levels get set as soon as JApp gets constructed XD

    std::cout << "Running JTest using sleeps:" << std::endl;
    {
        JApplication app;
        app.GetService<JLoggingService>()->set_level(JLogger::Level::OFF);
        app.AddPlugin("JTest");
        app.Run(false);
        std::cout << app.GetIntegratedRate() << std::endl;
        for (int nthreads=1; nthreads<=8; nthreads*=2) {
            app.Scale(nthreads);
            // Warm up for 10 secs
            std::this_thread::sleep_for(std::chrono::seconds(10));
            app.GetInstantaneousRate(); // Reset the clock
            std::this_thread::sleep_for(std::chrono::seconds(20));
            std::cout << nthreads << "    " << app.GetInstantaneousRate() << std::endl;
        }
        app.Stop(true);
        app.Join();
    }
    std::cout << "Running JTest using actual computations:" << std::endl;
    {
        for (int i=1; i<=8; i*=2) {
            std::cout << i << std::endl;
        }

    }
    std::cout << "Running JTest with all sleeps and computations turned off (pure overhead):" << std::endl;
    {
        for (int i=1; i<=8; i*=2) {
            std::cout << i << std::endl;
        }
    }

//    5Hz for full reconstruction
//    20kHz for stripped-down reconstruction (not per-core)
//    Base memory allcation: 100 MB/core + 600MB
//    1 thread/event, disentangle 1 event, turn into 40.
//    disentangled (single event size) : 12.5 kB / event (before blown up)
//    entangled "block of 40": dis * 40

}