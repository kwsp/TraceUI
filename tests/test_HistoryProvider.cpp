#include <QtTest>
#include <QGuiApplication>
#include <QSignalSpy>
#include "backend/ScrollingHistoryProvider.h"
#include "backend/CpuHistoryProvider.h"
#include "backend/NetworkHistoryProvider.h"

// Test harness that exposes protected members of ScrollingHistoryProvider
class TestableHistoryProvider : public ScrollingHistoryProvider {
  Q_OBJECT
public:
  using ScrollingHistoryProvider::ScrollingHistoryProvider;
  using ScrollingHistoryProvider::pushSample;
  using ScrollingHistoryProvider::history0;
  using ScrollingHistoryProvider::history1;
};

class HistoryProviderTest : public QObject {
  Q_OBJECT

private slots:
  // -- ScrollingHistoryProvider base class --

  void testInitialState() {
    TestableHistoryProvider provider;
    QCOMPARE(provider.phase(), 0.0F);
    QCOMPARE(provider.graphScale(), 1.0F);
    QVERIFY(provider.isTextureProvider());

    // History buffers should be zeroed
    for (int i = 0; i < ScrollingHistoryProvider::kSize; ++i) {
      QCOMPARE(provider.history0().at(i), 0.0F);
      QCOMPARE(provider.history1().at(i), 0.0F);
    }
  }

  void testPhaseAdvances() {
    TestableHistoryProvider provider;
    QSignalSpy phaseSpy(&provider, &ScrollingHistoryProvider::phaseChanged);

    provider.pushSample(1.0F, 2.0F, 100.0F);
    QCOMPARE(phaseSpy.count(), 1);
    QCOMPARE(provider.phase(), 1.0F / 128.0F);

    provider.pushSample(3.0F, 4.0F, 100.0F);
    QCOMPARE(phaseSpy.count(), 2);
    QCOMPARE(provider.phase(), 2.0F / 128.0F);
  }

  void testPhaseWraps() {
    TestableHistoryProvider provider;
    for (int i = 0; i < ScrollingHistoryProvider::kSize; ++i)
      provider.pushSample(1.0F, 1.0F, 100.0F);

    // After exactly kSize pushes, writeIndex wraps back to 0
    QCOMPARE(provider.phase(), 0.0F);
  }

  void testSamplesStored() {
    TestableHistoryProvider provider;
    provider.pushSample(42.0F, 99.0F, 100.0F);

    // First sample written at index 0
    QCOMPARE(provider.history0().at(0), 42.0F);
    QCOMPARE(provider.history1().at(0), 99.0F);
  }

  void testCircularOverwrite() {
    TestableHistoryProvider provider;

    // Fill the entire buffer
    for (int i = 0; i < ScrollingHistoryProvider::kSize; ++i)
      provider.pushSample(static_cast<float>(i), static_cast<float>(i * 2),
                          100.0F);

    // Overwrite index 0
    provider.pushSample(999.0F, 888.0F, 100.0F);
    QCOMPARE(provider.history0().at(0), 999.0F);
    QCOMPARE(provider.history1().at(0), 888.0F);

    // Index 1 still has the original value
    QCOMPARE(provider.history0().at(1), 1.0F);
  }

  void testGraphScaleStaysAtOne() {
    TestableHistoryProvider provider;
    QSignalSpy scaleSpy(&provider,
                        &ScrollingHistoryProvider::graphScaleChanged);

    // graphScale starts at 1.0 and pushSample keeps it at 1.0
    provider.pushSample(50.0F, 50.0F, 100.0F);
    QCOMPARE(provider.graphScale(), 1.0F);
    QCOMPARE(scaleSpy.count(), 0);
  }

  // -- CpuHistoryProvider --

  void testCpuProviderStoresSamples() {
    CpuHistoryProvider cpu;
    QSignalSpy phaseSpy(&cpu, &ScrollingHistoryProvider::phaseChanged);

    cpu.onDataUpdated(25.5, 10.3);
    QCOMPARE(phaseSpy.count(), 1);
    QVERIFY(cpu.phase() > 0.0F);
  }

  void testCpuProviderMultipleSamples() {
    CpuHistoryProvider cpu;
    for (int i = 0; i < 10; ++i)
      cpu.onDataUpdated(static_cast<double>(i), static_cast<double>(i) * 0.5);

    QCOMPARE(cpu.phase(), 10.0F / 128.0F);
    QCOMPARE(cpu.graphScale(), 1.0F);
  }

  // -- NetworkHistoryProvider --

  void testNetworkProviderStoresSamples() {
    NetworkHistoryProvider net;
    QSignalSpy phaseSpy(&net, &ScrollingHistoryProvider::phaseChanged);

    net.onDataUpdated(1024.0, 512.0);
    QCOMPARE(phaseSpy.count(), 1);
    QVERIFY(net.phase() > 0.0F);
  }

  void testNetworkProviderMultipleSamples() {
    NetworkHistoryProvider net;
    for (int i = 0; i < 20; ++i)
      net.onDataUpdated(static_cast<double>(i) * 100.0,
                        static_cast<double>(i) * 50.0);

    QCOMPARE(net.phase(), 20.0F / 128.0F);
    QCOMPARE(net.graphScale(), 1.0F);
  }

  // -- Shared behavior --

  void testTextureProviderNotNull() {
    CpuHistoryProvider cpu;
    QVERIFY(cpu.textureProvider() != nullptr);

    NetworkHistoryProvider net;
    QVERIFY(net.textureProvider() != nullptr);
  }
};

QTEST_MAIN(HistoryProviderTest)
#include "test_HistoryProvider.moc"
