
#include <catch.hpp>
#include <JANA/Engine/JUnfoldArrow.h>

namespace jana {
namespace unfoldtests {

using EventT = std::shared_ptr<JEvent>;


struct TestUnfolder : public JEventUnfolder {
    mutable std::vector<int> preprocessed_event_nrs;
    mutable std::vector<JEventLevel> preprocessed_event_levels;
    std::vector<int> unfolded_parent_nrs;
    std::vector<JEventLevel> unfolded_parent_levels;
    std::vector<int> unfolded_child_nrs;
    std::vector<JEventLevel> unfolded_child_levels;

    void Preprocess(const JEvent& parent) const override {
        LOG << "Preprocessing " << parent.GetLevel() << " event " << parent.GetEventNumber() << LOG_END;
        preprocessed_event_nrs.push_back(parent.GetEventNumber());
        preprocessed_event_levels.push_back(parent.GetLevel());
    }

    Result Unfold(const JEvent& parent, JEvent& child, int iter) override {
        auto child_nr = iter + 100 + parent.GetEventNumber();
        unfolded_parent_nrs.push_back(parent.GetEventNumber());
        unfolded_parent_levels.push_back(parent.GetLevel());
        unfolded_child_nrs.push_back(child_nr);
        unfolded_child_levels.push_back(child.GetLevel());
        child.SetEventNumber(child_nr);
        LOG << "Unfolding " << parent.GetLevel() << " event " << parent.GetEventNumber() << " into " << child.GetLevel() << " " << child_nr << "; iter=" << iter << LOG_END;
        return (iter == 2 ? Result::Finished : Result::KeepGoing);
    }
};

TEST_CASE("UnfoldTests_Basic") {

    JApplication app;
    auto jcm = app.GetService<JComponentManager>();

    JEventPool parent_pool {jcm, 5, 1, true, JEventLevel::Timeslice}; // size=5, locations=1, limit_total_events_in_flight=true
    JEventPool child_pool {jcm, 5, 1, true, JEventLevel::Event};
    JMailbox<EventT*> parent_queue {3}; // size=2
    JMailbox<EventT*> child_queue {3};

    parent_pool.init();
    child_pool.init();

    auto evt1 = parent_pool.get();
    (*evt1)->SetEventNumber(17);
    (*evt1)->SetLevel(JEventLevel::Timeslice);

    auto evt2 = parent_pool.get();
    (*evt2)->SetEventNumber(28);
    (*evt2)->SetLevel(JEventLevel::Timeslice);

    parent_queue.try_push(&evt1, 1);
    parent_queue.try_push(&evt2, 1);

    TestUnfolder unfolder;
    JUnfoldArrow arrow("sut", &unfolder, &parent_queue, &child_pool, &child_queue);

    JArrowMetrics m;
    arrow.initialize();
    arrow.execute(m, 0);
    REQUIRE(m.get_last_status() == JArrowMetrics::Status::KeepGoing);
    REQUIRE(child_queue.size() == 1);
    REQUIRE(unfolder.preprocessed_event_nrs.size() == 0);
    REQUIRE(unfolder.unfolded_parent_nrs.size() == 1);
    REQUIRE(unfolder.unfolded_parent_nrs[0] == 17);
    REQUIRE(unfolder.unfolded_parent_levels[0] == JEventLevel::Timeslice);
    REQUIRE(unfolder.unfolded_child_nrs.size() == 1);
    REQUIRE(unfolder.unfolded_child_nrs[0] == 117);
    REQUIRE(unfolder.unfolded_child_levels[0] == JEventLevel::Event);

}

    
} // namespace arrowtests
} // namespace jana



