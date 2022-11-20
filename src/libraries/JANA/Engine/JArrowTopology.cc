
// Copyright 2020, Jefferson Science Associates, LLC.
// Subject to the terms in the LICENSE file found in the top-level directory.


#include "JArrowTopology.h"
#include "JEventProcessorArrow.h"
#include "JEventSourceArrow.h"

JArrowTopology::JArrowTopology() = default;

JArrowTopology::~JArrowTopology() {
    LOG_DEBUG(m_logger) << "JArrowTopology: Entering destructor" << LOG_END;
    // finish(); // We don't want to call finish() here in case there was an exception in JArrow::initialize(), finalize()

    for (auto arrow : arrows) {
        delete arrow;
    }
    for (auto queue : queues) {
        delete queue;
    }
}

std::ostream& operator<<(std::ostream& os, JArrowTopology::Status status) {
    switch(status) {
        case JArrowTopology::Status::Uninitialized: os << "Uninitialized"; break;
        case JArrowTopology::Status::Running: os << "Running"; break;
        case JArrowTopology::Status::Pausing: os << "Pausing"; break;
        case JArrowTopology::Status::Paused: os << "Paused"; break;
        case JArrowTopology::Status::Finished: os << "Finished"; break;
        case JArrowTopology::Status::Draining: os << "Draining"; break;
    }
    return os;
}


/// This needs to be called _before_ launching the worker threads. After this point, everything is initialized.
/// No initialization happens afterwards.
void JArrowTopology::initialize() {
    assert(m_current_status == Status::Uninitialized);
    for (JArrow* arrow : arrows) {
        LOG_DEBUG(m_logger) << "Initializing arrow '" << arrow->get_name() << "'" << LOG_END;
        arrow->initialize();
    }
    m_current_status = Status::Paused;
}

void JArrowTopology::drain() {
    if (m_current_status != Status::Running) {
        LOG_DEBUG(m_logger) << "request_drain(): Skipping drain() because topology is not running" << LOG_END;
        return;
    }
    LOG_INFO(m_logger) << "Pausing all sources and draining queues..." << LOG_END;
    for (auto source : sources) {
        source->pause();
        m_current_status = Status::Draining;
    }
}

void JArrowTopology::run(int nthreads) {

    Status current_status = m_current_status;
    if (current_status == Status::Running || current_status == Status::Finished) {
        LOG_DEBUG(m_logger) << "JArrowTopology: run() : " << current_status << " => " << current_status << LOG_END;
        return;
    }
    LOG_DEBUG(m_logger) << "JArrowTopology: run() : " << current_status << " => Running" << LOG_END;

    if (sources.empty()) {
        throw JException("No event sources found!");
    }
    for (auto source : sources) {
        // We activate any sources, and everything downstream activates automatically
        // Note that this won't affect finished sources. It will, however, stop drain().
        source->run();
    }
    // Note that we activate workers AFTER we activate the topology, so no actual processing will have happened
    // by this point when we start up the metrics.
    metrics.reset();
    metrics.start(nthreads);
    m_current_status = Status::Running;
}

void JArrowTopology::request_pause() {
    // This sets all Running arrows to Paused, which prevents Workers from picking up any additional assignments
    // Once all Workers have completed their remaining assignments, the scheduler will notify us via achieve_pause().
    Status current_status = m_current_status;
    if (current_status != Status::Running) {
        LOG_DEBUG(m_logger) << "Ignoring request_pause(): Status is " << current_status << LOG_END;
    }
    else {
        LOG_INFO(m_logger) << "Pausing execution immediately without draining queues..." << LOG_END;
        for (auto arrow: arrows) {
            arrow->pause(); // If arrow is not running, pause() is a no-op
        }
        m_current_status = Status::Pausing;
    }
}

void JArrowTopology::achieve_pause() {
    // This is meant to be used by the scheduler to tell us when all workers have stopped, so it is safe to finish(), etc
    Status current_status = m_current_status;
    if (current_status == Status::Running || current_status == Status::Pausing || current_status == Status::Draining) {
        LOG_INFO(m_logger) << "Topology has successfully paused." << LOG_END;
        metrics.stop();
        m_current_status = Status::Paused;
    }
    else {
        LOG_DEBUG(m_logger) << "Ignoring achieve_pause(): Status is " << current_status << LOG_END;
    }
}

/// Finish is called by a single thread once the worker threads have all joined.
void JArrowTopology::finish() {
    // This finalizes all arrows. Once this happens, we cannot restart the topology.
    // assert(m_current_status == Status::Paused);
    for (auto arrow : arrows) {
        arrow->finish();
    }
    m_current_status = Status::Finished;
}

