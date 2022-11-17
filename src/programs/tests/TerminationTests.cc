
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
        auto source = new InterruptedSource("InterruptedSource", &app);
        app.Add(source);
        app.Run(true);
        REQUIRE(processor->processed_count == 0);
        REQUIRE(processor->finish_call_count == 0);
        // Stop() tells JApplication to finish Initialize() but not to proceed with Run().
        // If we had called Quit() instead, it would have exited Initialize() immediately and ended the program.

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
        REQUIRE(source->event_count > 0);
        REQUIRE(app.GetNEventsProcessed() == source->event_count);
        REQUIRE(processor->finish_call_count == 1);
    }

};


