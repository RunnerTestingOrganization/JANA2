
// Copyright 2020, Jefferson Science Associates, LLC.
// Subject to the terms in the LICENSE file found in the top-level directory.

#include "TerminationTests.h"
#include "catch.hpp"
#include "JANA/Engine/JArrowProcessingController.h"
#include <thread>

#include <JANA/JApplication.h>
#include <JANA/Engine/JArrowTopology.h>




TEST_CASE("TerminationTests") {

    auto parms = new JParameterManager;
    // parms->SetParameter("log:debug","JScheduler,JArrowProcessingController,JWorker,JArrow");
    JApplication app(parms);
    auto processor = new CountingProcessor(&app);
    app.Add(processor);
    app.SetParameterValue("jana:extended_report", 0);

    SECTION("Arrow engine, manual termination") {

        app.SetParameterValue("jana:engine", 0);
        auto source = new UnboundedSource("UnboundedSource", &app);
        app.Add(source);
        app.Run(false);
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        app.Quit(true);
        app.Join();
        REQUIRE(source->event_count > 0);
        REQUIRE(processor->finish_call_count == 1);
        REQUIRE(app.GetNEventsProcessed() == source->event_count);
    }

    SECTION("Arrow engine, self termination") {

        app.SetParameterValue("jana:engine", 0);
        auto source = new BoundedSource("BoundedSource", &app);
        app.Add(source);
        app.Run(true);
        REQUIRE(source->event_count == 10);
        REQUIRE(processor->processed_count == 10);
        REQUIRE(processor->finish_call_count == 1);
        REQUIRE(app.GetNEventsProcessed() == source->event_count);
    }

    SECTION("Arrow engine, interrupted during JEventSource::Open()") {

        app.SetParameterValue("jana:engine", 0);
        app.SetParameterValue("nthreads", 4);
        auto source = new InterruptedSource("InterruptedSource", &app, true, false, false);
        app.Add(source);
        app.Run(true);
        REQUIRE(processor->processed_count == 0);
        REQUIRE(processor->finish_call_count == 0);
        // Quit() tells JApplication to finish Initialize() but not to proceed with Run().
        // If we had called Quit() instead, it would have exited Initialize() immediately and ended the program.

        REQUIRE(app.GetExitCode() == (int) JApplication::ExitCode::Success);
        REQUIRE(app.GetNEventsProcessed() == source->GetEventCount());
    }

    SECTION("Debug engine, self-termination") {

        app.SetParameterValue("jana:engine", 1);
        auto source = new BoundedSource("BoundedSource", &app);
        app.Add(source);
        app.Run(true);
        REQUIRE(source->event_count == 10);
        REQUIRE(processor->processed_count == 10);
        REQUIRE(processor->finish_call_count == 1);
        REQUIRE(app.GetNEventsProcessed() == source->event_count);
    }

    SECTION("Debug engine, manual termination") {

        app.SetParameterValue("jana:engine", 1);
        auto source = new UnboundedSource("UnboundedSource", &app);
        app.Add(source);
        app.Run(false);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        app.Quit(true);
        app.Join();
        REQUIRE(source->event_count > 0);
        REQUIRE(app.GetNEventsProcessed() == source->event_count);
        REQUIRE(processor->finish_call_count == 1);
    }

};


void test_internal_termination(bool call_quit_vs_pause, bool interrupt_open_vs_getevent, bool drain, int nthreads) {

    std::ostringstream oss;
    oss << "Internal termination via "
        << (call_quit_vs_pause ? "Quit(" : "Pause(")
        << (drain ? "drain=true)" : "drain=false") << " in "
        << (interrupt_open_vs_getevent ? "Open()" : "GetEvent()") << " with nthreads=" << nthreads;

    SECTION(oss.str()) {

        auto parms = new JParameterManager;
        parms->SetParameter("log:debug","JScheduler,JArrowProcessingController,JWorker,JArrow,JArrowTopology");
        JApplication app(parms);
        auto processor = new CountingProcessor(&app);
        app.Add(processor);
        app.SetParameterValue("jana:extended_report", 0);
        app.SetParameterValue("jana:engine", 0);
        app.SetParameterValue("nthreads", nthreads);
        app.SetParameterValue("jana:event_source_chunksize", 1); // Use this to control number of events that initially get emitted
        auto source = new InterruptedSource("InterruptedSource", &app, interrupt_open_vs_getevent, call_quit_vs_pause, drain);
        app.Add(source);
        app.Run(true);

        // Should never time out
        REQUIRE(app.GetExitCode() == (int) JApplication::ExitCode::Success);

        if (interrupt_open_vs_getevent) {
            // Interrupting Open() means no events should get emitted
            REQUIRE(source->GetEventCount() == 0);
        }
        else {
            // Interrupting GetEvent() on 4th event means no more events should have been emitted
            REQUIRE(source->GetEventCount() == 4);
        }

        if (drain) {
            // Draining queues means that each emitted event gets processed
            REQUIRE(app.GetNEventsProcessed() == source->GetEventCount());
            REQUIRE(processor->processed_count == source->GetEventCount());
        }

        if (call_quit_vs_pause) {
            // Because we called Quit(), Run(true) should have called finish()
            REQUIRE(processor->finish_call_count == 1);
        }
        else {
            // Because we called Pause(), Run(true) should have exited without calling finish()
            REQUIRE(processor->finish_call_count == 0);
        }
    }
}


TEST_CASE("InternalTerminationTests") {

    // test_internal_termination(bool call_quit_vs_pause, bool interrupt_open_vs_getevent, bool drain, int nthreads)
    test_internal_termination(true, true, true, 1);
    test_internal_termination(true, true, true, 7);
    test_internal_termination(true, true, false, 1);
    test_internal_termination(true, true, false, 7);
    test_internal_termination(true, false, true, 1);
    test_internal_termination(true, false, true, 7);
    test_internal_termination(true, false, false, 1);
    test_internal_termination(true, false, false, 7);
    test_internal_termination(false, true, true, 1);
    test_internal_termination(false, true, true, 7);
    test_internal_termination(false, true, false, 1);
    test_internal_termination(false, true, false, 7);
    test_internal_termination(false, false, true, 1);
    test_internal_termination(false, false, true, 7);
    test_internal_termination(false, false, false, 1);
    test_internal_termination(false, false, false, 7);
}