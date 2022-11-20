
// Copyright 2020, Jefferson Science Associates, LLC.
// Subject to the terms in the LICENSE file found in the top-level directory.

#include <JANA/Engine/JScheduler.h>
#include <JANA/Engine/JArrowTopology.h>
#include <JANA/Services/JLoggingService.h>


JScheduler::JScheduler(std::shared_ptr<JArrowTopology> topology)
    : m_topology(topology)
    , m_next_idx(0)
    {
    }


JArrow* JScheduler::next_assignment(uint32_t worker_id, JArrow* assignment, JArrowMetrics::Status last_result) {

    std::lock_guard<std::mutex> lock(m_mutex);

    // Check latest arrow back in
    if (assignment != nullptr) {
        assignment->update_thread_count(-1);

        if (assignment->get_running_upstreams() == 0 &&
            assignment->get_pending() == 0 &&
            assignment->get_thread_count() == 0 &&
            assignment->get_type() != JArrow::NodeType::Source &&
                assignment->get_status() == JArrow::Status::Running) {

            LOG_DEBUG(logger) << "Deactivating arrow '" << assignment->get_name() << "' (" << m_topology->running_arrow_count - 1 << " remaining)" << LOG_END;
            assignment->pause();
            assert(m_topology->running_arrow_count >= 0);
            if (m_topology->running_arrow_count == 0) {
                LOG_DEBUG(logger) << "All arrows deactivated. Deactivating topology." << LOG_END;
                m_topology->achieve_pause();
            }
        }
    }

    if (m_topology->m_current_status == JArrowTopology::Status::Paused) {
        // If the topology has already deactivated, workers can exit immediately
        return nullptr;
    }

    // Choose a new arrow. Loop over all arrows, starting at where we last left off, and pick the first
    // arrow that works
    size_t current_idx = m_next_idx;
    do {
        JArrow* candidate = m_topology->arrows[current_idx];
        current_idx += 1;
        current_idx %= m_topology->arrows.size();

        if (candidate->get_status() == JArrow::Status::Running &&
            (candidate->is_parallel() || candidate->get_thread_count() == 0)) {

            // Found a plausible candidate.

            if (candidate->get_type() == JArrow::NodeType::Source ||
                candidate->get_running_upstreams() > 0 ||
                candidate->get_pending() > 0) {

                // Candidate still has work they can do
                m_next_idx = current_idx; // Next time, continue right where we left off
                candidate->update_thread_count(1);

                LOG_DEBUG(logger) << "Worker " << worker_id << ", "
                                  << ((assignment == nullptr) ? "idle" : assignment->get_name())
                                  << ", " << to_string(last_result) << " => "
                                  << candidate->get_name() << "  [" << candidate->get_thread_count() << " threads]" << LOG_END;
                return candidate;

            }
            else {
                // Candidate can be paused immediately because there is no more work coming
                LOG_DEBUG(logger) << "Deactivating arrow '" << candidate->get_name() << "' (" << m_topology->running_arrow_count - 1 << " remaining)" << LOG_END;
                candidate->pause();
                assert(m_topology->running_arrow_count >= 0);
                if (m_topology->running_arrow_count == 0) {
                    LOG_DEBUG(logger) << "All arrows deactivated. Deactivating topology..." << LOG_END;
                    m_topology->achieve_pause();
                }
            }
        }

    } while (current_idx != m_next_idx);

    if (m_topology->running_arrow_count == 0) {
        // This exists in case
        // a) The user provided a topology that cannot self-exit, e.g. because no event sources have been specified
        // b) The user called JApp::Pause(), so that all arrows were paused immediately
        LOG_DEBUG(logger) << "All arrows deactivated. Deactivating topology..." << LOG_END;
        m_topology->achieve_pause();
    }
    return nullptr;  // We've looped through everything with no luck
}


void JScheduler::last_assignment(uint32_t worker_id, JArrow* assignment, JArrowMetrics::Status result) {

    LOG_DEBUG(logger) << "Worker " << worker_id << ", "
                       << ((assignment == nullptr) ? "idle" : assignment->get_name())
                       << ", " << to_string(result) << ") => Shutting down!" << LOG_END;

    std::lock_guard<std::mutex> lock(m_mutex);
    if (assignment != nullptr) {
        assignment->update_thread_count(-1);
    }
}



