/*
 * Copyright (c) 2026 TULPAR project team
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */

#include "ns3/test.h"
#include "ns3/tulpar-scenario.h"

using namespace ns3;
using namespace ns3::tulpar;

/**
 * @defgroup tulpar-tests TULPAR tests
 * @ingroup tulpar
 * @ingroup tests
 */

/**
 * @ingroup tulpar-tests
 * RSD-SIM-01d: two runs of the same configuration with the same seed give
 * identical event logs and trace hashes.
 */
class TulparSameSeedTestCase : public TestCase
{
  public:
    TulparSameSeedTestCase()
        : TestCase("RSD-SIM-01d same seed gives identical event logs")
    {
    }

  private:
    void DoRun() override
    {
        ScenarioConfig config;
        config.seed = 7;
        config.run = 3;
        config.durationS = 30;
        RunResult a = RunScenario(config);
        RunResult b = RunScenario(config);
        NS_TEST_ASSERT_MSG_GT(a.tx, 0, "the scenario sent no beacon");
        NS_TEST_ASSERT_MSG_GT(a.rx, 0, "the scenario received no beacon");
        NS_TEST_ASSERT_MSG_EQ(a.traceHash, b.traceHash, "trace hashes differ");
        NS_TEST_ASSERT_MSG_EQ((a.eventLog == b.eventLog), true, "event logs differ");
    }
};

/**
 * @ingroup tulpar-tests
 * RSD-SIM-01c: the random streams follow the run number, so another run number
 * gives another event log.
 */
class TulparRunNumberTestCase : public TestCase
{
  public:
    TulparRunNumberTestCase()
        : TestCase("RSD-SIM-01c run number changes the random streams")
    {
    }

  private:
    void DoRun() override
    {
        ScenarioConfig config;
        config.durationS = 30;
        config.run = 1;
        RunResult a = RunScenario(config);
        config.run = 2;
        RunResult b = RunScenario(config);
        NS_TEST_ASSERT_MSG_NE(a.traceHash, b.traceHash, "run numbers 1 and 2 gave the same log");
    }
};

/**
 * @ingroup tulpar-tests
 * RSD-SIM-02b (hidden-label part): runs that differ only in the hidden attacker
 * label give identical agent event logs, while the ground-truth logs differ.
 */
class TulparHiddenLabelTestCase : public TestCase
{
  public:
    TulparHiddenLabelTestCase()
        : TestCase("RSD-SIM-02b hidden attacker label changes no agent event")
    {
    }

  private:
    void DoRun() override
    {
        ScenarioConfig config;
        config.durationS = 30;
        config.attacker = "none";
        RunResult a = RunScenario(config);
        config.attacker = "UAV-1";
        RunResult b = RunScenario(config);
        NS_TEST_ASSERT_MSG_EQ((a.groundTruthLog != b.groundTruthLog),
                              true,
                              "the label did not reach the ground truth");
        NS_TEST_ASSERT_MSG_EQ(a.traceHash, b.traceHash, "the hidden label changed agent events");
        NS_TEST_ASSERT_MSG_EQ((a.eventLog == b.eventLog), true, "event logs differ");
    }
};

/**
 * @ingroup tulpar-tests
 * Gate G0 test suite.
 */
class TulparG0TestSuite : public TestSuite
{
  public:
    TulparG0TestSuite()
        : TestSuite("tulpar-g0", Type::UNIT)
    {
        AddTestCase(new TulparSameSeedTestCase, Duration::QUICK);
        AddTestCase(new TulparRunNumberTestCase, Duration::QUICK);
        AddTestCase(new TulparHiddenLabelTestCase, Duration::QUICK);
    }
};

static TulparG0TestSuite g_tulparG0TestSuite; ///< static instance of the suite
